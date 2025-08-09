#pragma once

#include <string>
#include <functional>

namespace ZipUtils {
    
    /**
     * Progress callback function type
     * Parameters: current_file_index, total_files, current_file_path, bytes_processed, total_bytes
     */
    using ProgressCallback = std::function<void(size_t, size_t, const std::string&, size_t, size_t)>;
    
    /**
     * Zips a directory with optional deletion of source content and progress reporting
     * 
     * @param directory_path Path to the directory to zip
     * @param zip_name Name/path for the output ZIP file
     * @param delete_source If true, deletes the source directory after successful zipping
     * @param progress_callback Optional callback function for progress updates
     * @return true if successful, false otherwise
     * @throws std::runtime_error if directory doesn't exist or ZIP creation fails
     */
    bool zip_directory(
        const std::string& directory_path, 
        const std::string& zip_name, 
        bool delete_source = false,
        ProgressCallback progress_callback = nullptr
    );
    
    /**
     * Overloaded version with just directory path and zip name
     */
    bool zip_directory(const std::string& directory_path, const std::string& zip_name);
    
    /**
     * Overloaded version with progress callback but no source deletion
     */
    bool zip_directory(
        const std::string& directory_path, 
        const std::string& zip_name, 
        ProgressCallback progress_callback
    );
}
