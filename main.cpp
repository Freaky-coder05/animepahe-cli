#include <cxxopts.hpp>
#include <fmt/core.h>
#include <fmt/color.h>
#include <string>
#include <utils.hpp>
#include <animepahe.hpp>

using namespace AnimepaheCLI;

int main(int argc, char *argv[])
{
    /**
     * -l, --link
     * input anime series link or a single episode link
     * -e, --episodes
     * all,1,1-15 all means full series, or episode range
     * -x, --export
     * saves all download links to a text file
     * -q, --quality
     * set target quality, if available it will select otherwise fall back to maximum
     * -z, --zip
     * creates a zip from downloaded items */

    cxxopts::Options options("animepahe-cli", "AnimePahe CLI Downloader");
    options.add_options()
    ("l,link", "Input anime series link or a single episode link", cxxopts::value<std::string>())
    ("e,episodes", "Specify episodes to download (all, 1-15)", cxxopts::value<std::string>())
    ("q,quality", "Set target quality", cxxopts::value<int>()->default_value("0"))
    ("x,export", "Export download links to a text file", cxxopts::value<bool>()->default_value("false"))
    ("z,zip", "Create a zip from downloaded items", cxxopts::value<bool>()->default_value("false"))
    ("h,help", "Print usage");

    try
    {
        auto result = options.parse(argc, argv);
        if (result.count("help"))
        {
            fmt::print("{}\n", options.help());
            return 0;
        }

        std::string link = result["link"].as<std::string>();
        std::string episodes = result["episodes"].as<std::string>();
        int targetRes = result["quality"].as<int>();
        bool exportLinks = result["export"].as<bool>();
        bool createZip = result["zip"].as<bool>();

        if (!isFullSeriesURL(link) && !isEpisodeURL(link))
        {
            throw std::runtime_error("Invalid link format. Please provide a valid AnimePahe series or episode link.");
        }
        if (!isValidEpisodeRangeFormat(episodes))
        {
            throw std::runtime_error("Invalid episode range format. Use 'all' or '1-15'.");
        }

        fmt::print("\n * Animepahe-CLI (v0.1.4-beta) https://github.com/Danushka-Madushan/animepahe-cli \n");

        // Create an instance of Animepahe and call the extractor method
        Animepahe animepahe;
        animepahe.extractor(
            isFullSeriesURL(link),
            link,
            targetRes,
            episodes == "all",
            episodes == "all" ? std::vector<int>() : parseEpisodeRange(episodes),
            exportLinks,
            createZip
        );
    }
    catch (const cxxopts::exceptions::option_has_no_value)
    {
        fmt::print("\n --link,-l and --episodes,-e must be provided!\n\n");
        return 1;
    }
    catch (const cxxopts::exceptions::missing_argument)
    {
        fmt::print("\n Usage: -l,--link \"https://animepahe.ru/anime/....\" -e,--episodes [all,1-12] -q,--quality 720 -x,--export -z,--zip\n\n");
        return 1;
    }
    catch (const std::runtime_error &e)
    {
        fmt::print("\n\n * ");
        fmt::print(fmt::fg(fmt::color::indian_red), "ERROR :");
        fmt::print(" {} \n\n", e.what());
        return 1;
    }
    catch (const std::exception &e)
    {
        fmt::print("\n\n * ");
        fmt::print(fmt::fg(fmt::color::indian_red), "ERROR :");
        fmt::print(" {} \n\n", e.what());
        return 1;
    }

    return 0;
}
