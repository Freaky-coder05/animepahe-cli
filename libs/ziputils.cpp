#include "ziputils.hpp"
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <zip.h>

namespace ZipUtils {
    
    bool zip_directory(
        const std::string& directory_path, 
        const std::string& zip_name, 
        bool delete_source,
        ProgressCallback progress_callback
    ) {
        namespace fs = std::filesystem;
        
        /* Check if source directory exists */ 
        if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
            throw std::runtime_error("Directory does not exist: " + directory_path);
        }
        
        // First pass: count files and calculate total size for progress tracking
        std::vector<fs::directory_entry> entries;
        size_t total_bytes = 0;
        
        for (const auto& entry : fs::recursive_directory_iterator(directory_path)) {
            entries.push_back(entry);
            if (entry.is_regular_file()) {
                std::error_code ec;
                auto size = fs::file_size(entry.path(), ec);
                if (!ec) {
                    total_bytes += size;
                }
            }
        }
        
        // Create ZIP file
        struct zip_t *zip = zip_open(zip_name.c_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
        if (!zip) {
            throw std::runtime_error("Failed to create ZIP file: " + zip_name);
        }
        
        try {
            // Get the base directory name for relative paths
            fs::path base_path(directory_path);
            size_t current_file_index = 0;
            size_t bytes_processed = 0;
            
            // Process all files and directories
            for (const auto& entry : entries) {
                // Get relative path from base directory
                fs::path relative_path = fs::relative(entry.path(), base_path);
                std::string zip_entry_path = relative_path.string();
                
                // Convert Windows backslashes to forward slashes for ZIP compatibility
                std::replace(zip_entry_path.begin(), zip_entry_path.end(), '\\', '/');
                
                // Report progress before processing each entry
                if (progress_callback) {
                    progress_callback(current_file_index, entries.size(), entry.path().string(), bytes_processed, total_bytes);
                }
                
                if (entry.is_directory()) {
                    // Add directory entry (with trailing slash)
                    zip_entry_path += "/";
                    if (zip_entry_open(zip, zip_entry_path.c_str()) < 0) {
                        zip_close(zip);
                        throw std::runtime_error("Failed to add directory to ZIP: " + zip_entry_path);
                    }
                    zip_entry_close(zip);
                } else if (entry.is_regular_file()) {
                    // Add file entry
                    if (zip_entry_open(zip, zip_entry_path.c_str()) < 0) {
                        zip_close(zip);
                        throw std::runtime_error("Failed to open ZIP entry: " + zip_entry_path);
                    }
                    
                    if (zip_entry_fwrite(zip, entry.path().string().c_str()) < 0) {
                        zip_entry_close(zip);
                        zip_close(zip);
                        throw std::runtime_error("Failed to write file to ZIP: " + entry.path().string());
                    }
                    
                    // Update bytes processed
                    std::error_code ec;
                    auto file_size = fs::file_size(entry.path(), ec);
                    if (!ec) {
                        bytes_processed += file_size;
                    }
                    
                    zip_entry_close(zip);
                }
                
                current_file_index++;
            }
            
            // Final progress update
            if (progress_callback) {
                progress_callback(entries.size(), entries.size(), "Compression complete", total_bytes, total_bytes);
            }
            
            // Close ZIP file
            zip_close(zip);
            
            // Delete source directory if requested
            if (delete_source) {
                std::error_code ec;
                fs::remove_all(directory_path, ec);
                if (ec) {
                    throw std::runtime_error("Failed to delete source directory: " + ec.message());
                }
            }
            
            return true;
            
        } catch (...) {
            // Clean up ZIP file on error
            zip_close(zip);
            throw;
        }
    }
    
    bool zip_directory(const std::string& directory_path, const std::string& zip_name) {
        return zip_directory(directory_path, zip_name, false, nullptr);
    }
    
    bool zip_directory(
        const std::string& directory_path, 
        const std::string& zip_name, 
        ProgressCallback progress_callback
    ) {
        return zip_directory(directory_path, zip_name, false, progress_callback);
    }
}
