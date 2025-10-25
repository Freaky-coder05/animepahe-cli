#include "downloader.hpp"
#include <fmt/core.h>
#include <fmt/color.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <regex>

Downloader::Downloader(const std::vector<std::string> &urls) : urls_(urls) {}

void Downloader::setDownloadDirectory(const std::string &dir)
{
    download_dir_ = dir;
    if (!std::filesystem::exists(download_dir_))
    {
        std::filesystem::create_directory(download_dir_);
    }
}


void Downloader::startDownloads()
{
    // Ensure "videos" folder exists inside download_dir_
    std::string videos_dir = download_dir_ + "/videos";
    if (!std::filesystem::exists(videos_dir))
        std::filesystem::create_directory(videos_dir);

    fmt::print("\n");
    for (const auto &url : urls_)
    {
        std::string filename = extractFilename(url);

        // Sanitize filename: replace any / or \ to prevent folder creation
        filename = std::regex_replace(filename, std::regex(R"([\\/])"), "_");

        std::string filepath = videos_dir + "/" + filename;

        fmt::print("\n * Downloading : ");
        fmt::print(fmt::fg(fmt::color::cyan), fmt::format("{}\n", filename));

        bool dlStatus = downloadFile(url, filepath);
        if (!dlStatus)
        {
            fmt::print("\n * DL (");
            fmt::print(fmt::fg(fmt::color::indian_red), "FAIL");
            fmt::print(")   : {}", url);
            std::filesystem::remove(filepath);
            continue;
        }
        else
        {
            // Move cursor up and clear line for clean output
            std::cout << "\x1b[1A";
            std::cout << "\x1b[2K\r";

            fmt::print(" * DL (");
            fmt::print(fmt::fg(fmt::color::lime_green), "DONE");
            fmt::print(")   : {}", filename);
        }
    }
}

std::string Downloader::extractFilename(const std::string &url) const
{
    /* Try to find "?file=" followed by the filename */
    std::regex filenamePattern(R"([\?&]file=([^&]+))");
    std::smatch match;

    if (std::regex_search(url, match, filenamePattern) && match.size() > 1)
    {
        return match[1];
    }

    /* If not found, generate a unique filename with timestamp */
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto ms_part = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

    std::ostringstream oss;
    oss << "file_" << std::put_time(std::localtime(&now_time), "%Y%m%d_%H%M%S") << "_" << ms_part << ".mp4";
    return oss.str();
}

std::string formatTime(double totalSeconds) {
    int seconds = static_cast<int>(std::round(totalSeconds)); /* Round to nearest second */
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << secs;

    return oss.str();
}

std::string formatSpeedMB(double speedKBps) {
    double mbps = speedKBps / (1024.0 * 1024.0);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2)
        << std::setw(4) << mbps << " MB/s";

    return oss.str();
}

auto formatSizeMB = [](size_t bytes) -> std::string {
    double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << mb << "MB";
    return oss.str();
};

bool Downloader::downloadFile(const std::string &url, const std::string &filepath)
{
    std::ofstream outfile(filepath, std::ios::binary);
    if (!outfile.is_open())
    {
        fmt::print("\n * Failed to open file: {}\n", filepath);
        return false;
    }

    auto start_time = std::chrono::steady_clock::now();
    std::string last_progress_line;
    
    cpr::Response r = cpr::Get(
        cpr::Url{url},
        cpr::WriteCallback{
            [&outfile](std::string data, intptr_t)
            {
                outfile.write(data.data(), data.size());
                return true;
            }},
        cpr::ProgressCallback{[&start_time, &last_progress_line](size_t downloadTotal, size_t downloadNow, size_t, size_t, intptr_t)
        {
            if (downloadTotal > 0)
            {
                double progress = static_cast<double>(downloadNow) / downloadTotal * 100.0;
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
                
                if (elapsed > 0)
                {
                    double speed = static_cast<double>(downloadNow) / elapsed;
                    double remaining = (downloadTotal - downloadNow) / speed;
                    
                    /* Build the complete progress string */
                    std::ostringstream progress_stream;
                    progress_stream << std::fixed << std::setprecision(2)
                    << " * Progress: " << progress
                    << "% ETA: " << formatTime(remaining)
                    << " | " << formatSpeedMB(speed)
                    << " | [" << formatSizeMB(downloadNow) << "/" << formatSizeMB(downloadTotal) << "]";
                    
                    std::string new_line = progress_stream.str();
                    
                    /* Clear the current progress line and rewrite it */
                    if (!last_progress_line.empty()) {
                        /*  Clear the previous progress line */
                        std::cout << "\r" << std::string(last_progress_line.length(), ' ') << "\r";
                    }
                    
                    std::cout << new_line << std::flush;
                    last_progress_line = new_line;
                }
            }
            return true;
        }
    });

    /* Clear the final progress line but leave cursor positioned for cleanup */
    if (!last_progress_line.empty()) {
        std::cout << "\r" << std::string(last_progress_line.length(), ' ') << "\r";
    }

    outfile.close();
    return r.status_code == 200;
}
