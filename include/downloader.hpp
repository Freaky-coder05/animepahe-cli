#include <cpr/cpr.h>
#include <filesystem>
#include <vector>
#include <string>

class Downloader {
public:
    Downloader(const std::vector<std::string>& urls);
    void setDownloadDirectory(const std::string& dir);
    void startDownloads();

private:
    std::vector<std::string> urls_;
    std::string download_dir_;

    std::string extractFilename(const std::string& url) const;
    bool downloadFile(const std::string& url, const std::string& filepath);
};
