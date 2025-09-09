#pragma once
#include <string>
#include <optional>

struct Release {
    std::string tag_name;
};

class GitHubUpdater {
public:
    GitHubUpdater(const std::string&, const std::string&, const std::string&, const std::string& = "") {}
    std::optional<Release> checkForUpdate() { return std::nullopt; }
    void checkAndUpdate(bool = false) {}
};
