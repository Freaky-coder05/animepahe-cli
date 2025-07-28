#include <animepahe.hpp>
#include <kwikpahe.hpp>
#include <cpr/cpr.h>
#include <re2/re2.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <utils.hpp>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace AnimepaheCLI
{
    cpr::Cookies cookies = cpr::Cookies{{"__ddg2_", ""}};

    cpr::Header Animepahe::getHeaders(const std::string &link)
    {
        const cpr::Header HEADERS = {
            {"accept", "application/json, text/javascript, */*; q=0.0"},
            {"accept-language", "en-US,en;q=0.9"},
            {"referer", link},
            {"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36 Edg/138.0.0.0"}};
        return HEADERS;
    }

    void Animepahe::extract_link_metadata(const std::string &link, bool isSeries)
    {
        fmt::print("\n\r * Requesting Info..");
        cpr::Response response = cpr::Get(
            cpr::Url{link},
            cpr::Header{getHeaders(link)}, cookies);

        fmt::print("\r * Requesting Info : ");
        
        if (response.status_code != 200)
        {
            fmt::print(fmt::fg(fmt::color::indian_red), "FAILED!\n");
            throw std::runtime_error(fmt::format("Failed to fetch {}, StatusCode: {}", link, response.status_code));
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::lime_green), "OK!\n");
        }

        RE2::GlobalReplace(&response.text, R"((\r\n|\r|\n))", "");

        if (isSeries)
        {
            re2::StringPiece TITLE_CONSUME = response.text;
            re2::StringPiece TYPE_CONSUME = response.text;
            re2::StringPiece EP_CONSUME = response.text;
            std::string title;
            std::string type;
            std::string episodesCount;

            if (RE2::FindAndConsume(&TITLE_CONSUME, R"re(style=[^=]+title="([^"]+)")re", &title))
            {
                title = unescape_html_entities(title);
            }

            if (RE2::FindAndConsume(&TYPE_CONSUME, R"re(Type:[^>]*title="[^"]*"[^>]*>([^<]+)</a>)re", &type))
            {
                type = unescape_html_entities(type);
            }

            if (RE2::FindAndConsume(&TYPE_CONSUME, R"re(Episode[^>]*>\s+(\d*)</p)re", &episodesCount))
            {
                episodesCount = unescape_html_entities(episodesCount);
            }

            fmt::print("\n * Anime: {}\n", title);
            fmt::print(" * Type: {}\n", type);
            fmt::print(" * Episodes: {}\n", episodesCount);
        }
        else
        {
            re2::StringPiece TITLE_TYPE_CONSUME = response.text;
            std::string title;
            std::string episode;

            if (RE2::FindAndConsume(&TITLE_TYPE_CONSUME, R"re(title="[^>]*>([^<]*)</a>\D*(\d*)<span)re", &title, &episode))
            {
                episode = unescape_html_entities(episode);
                title = unescape_html_entities(title);
            }

            fmt::print("\n * Anime: {}\n", title);
            fmt::print(" * Episode: {}\n", episode);
        }
    }

    std::map<std::string, std::string> Animepahe::fetch_episode(const std::string &link)
    {
        std::vector<std::map<std::string, std::string>> episodeData;
        cpr::Response response = cpr::Get(
            cpr::Url{link},
            cpr::Header{getHeaders(link)}, cookies);

        if (response.status_code != 200)
        {
            fmt::print("\n * Error: Failed to fetch {}, StatusCode {}\n", link, response.status_code);
            return {};
        }

        RE2::GlobalReplace(&response.text, R"((\r\n|\r|\n))", "");
        re2::StringPiece EP_CONSUME = response.text;
        std::string dPaheLink;
        std::string epName;

        while (RE2::FindAndConsume(&EP_CONSUME, R"re(href="(https://pahe\.win/\S*)"[^>]*>([^)]*\))[^<]*<)re", &dPaheLink, &epName))
        {
            std::map<std::string, std::string> content;
            content["dPaheLink"] = unescape_html_entities(dPaheLink);
            content["epName"] = unescape_html_entities(epName);

            re2::StringPiece RES_CONSUME = epName;
            std::string epRes;
            if (RE2::FindAndConsume(&RES_CONSUME, R"re(\b(\d{3,4})p\b)re", &epRes))
            {
                epRes = unescape_html_entities(epRes);
            }
            else
            {
                epRes = "0";
            }

            content["epRes"] = epRes;
            episodeData.push_back(content);
        }

        if (episodeData.empty())
        {
            throw std::runtime_error(fmt::format("\n No episodes found in {}", link));
        }

        /** 
         * Find the episode with the highest resolution
         * Since Animepahe sort JPN episodes at top this selects the highest resolution
         * JPN Episode and igore all others. btw who wants to watch anime in ENG anyway ?
        */
        std::map<std::string, std::string> *maxEpResMap = nullptr;
        int maxEpRes = 0;

        for (auto &episode : episodeData)
        {
            int epResValue = std::stoi(episode.at("epRes"));
            if (epResValue > maxEpRes)
            {
                maxEpRes = epResValue;
                maxEpResMap = &episode;
            }
        }
        
        return *maxEpResMap;
    }

    std::vector<std::string> Animepahe::fetch_series(const std::string &link)
    {
        std::vector<std::string> links;
        int page = 1; /* default is first page, but paging functionality need to be implement */
        /* extract session from url */
        std::string id;
        RE2::PartialMatch(link, R"(anime/([a-f0-9-]{36}))", &id);

        cpr::Response response = cpr::Get(
            cpr::Url{
                fmt::format("https://animepahe.ru/api?m=release&id={}&sort=episode_asc&page={}", id, page)},
            cpr::Header{getHeaders(link)}, cookies);

        if (response.status_code != 200)
        {
            throw std::runtime_error(fmt::format("\n * Error: Failed to fetch {}, StatusCode {}\n", link, response.status_code));
        }

        auto parsed = json::parse(response.text);

        if (parsed.contains("data") && parsed["data"].is_array())
        {
            for (const auto &episode : parsed["data"])
            {
                std::string session = episode.value("session", "unknown");
                std::string episodeLink = fmt::format("https://animepahe.ru/play/{}/{}", id, session);
                links.push_back(episodeLink);
            }
        }

        return links;
    }

    int Animepahe::get_series_episode_count(const std::string& link)
    {
        std::string id;
        RE2::PartialMatch(link, R"(anime/([a-f0-9-]{36}))", &id);

        cpr::Response response = cpr::Get(
            cpr::Url{
                fmt::format("https://animepahe.ru/api?m=release&id={}&sort=episode_asc&page={}", id, 1)},
            cpr::Header{getHeaders(link)}, cookies);

        if (response.status_code != 200)
        {
            throw std::runtime_error(fmt::format("\n * Error: Failed to fetch {}, StatusCode {}\n", link, response.status_code));
        }

        auto parsed = json::parse(response.text);
        int epCount = 0;
        if (parsed.contains("total") && parsed["total"].is_number_integer())
        {
            epCount = parsed["total"];
        }

        return epCount;
    }

    std::vector<std::map<std::string, std::string>> Animepahe::extract_link_content(
        const std::string &link,
        const std::vector<int> &episodes,
        bool isSeries,
        bool isAllEpisodes)
    {
        std::vector<std::map<std::string, std::string>> episodeListData;
        fmt::print("\n\r * Requesting Episodes.. ");

        if (isSeries)
        {
            std::vector<std::string> seriesEpLinks = fetch_series(link);
            if (isAllEpisodes)
            {
                for (int i = 0; i < seriesEpLinks.size(); ++i)
                {
                    const std::string &pLink = seriesEpLinks[i];
                    fmt::print("\r * Requesting Episode : EP{} ", padIntWithZero(i + 1));
                    std::map<std::string, std::string> epContent = fetch_episode(pLink);
                    fflush(stdout);
                    if (!epContent.empty())
                    {
                        episodeListData.push_back(epContent);
                    }
                }
            }
            else
            {
                for (int i = 0; i < seriesEpLinks.size(); ++i)
                {
                    const std::string &pLink = seriesEpLinks[i];
                    if (episodes[0] > seriesEpLinks.size() || episodes[1] > seriesEpLinks.size())
                    {
                        throw std::runtime_error(fmt::format("Invalid episode range: {}-{} for series with {} episodes", episodes[0], episodes[1], seriesEpLinks.size()));
                    }

                    if ((i >= episodes[0] - 1 && i <= episodes[1] - 1))
                    {
                        fmt::print("\r * Requesting Episode : EP{} ", padIntWithZero(i + 1));
                        std::map<std::string, std::string> epContent = fetch_episode(pLink);
                        fflush(stdout);
                        if (!epContent.empty())
                        {
                            episodeListData.push_back(epContent);
                        }
                    }
                }
            }
        }
        else
        {

            std::map<std::string, std::string> epContent = fetch_episode(link);
            if (epContent.empty())
            {
                fmt::print("\n * Error: No episode data found for {}\n", link);
                return {};
            }

            episodeListData.push_back(epContent);
        }

        fmt::print("\r * Requesting Episodes : {} ", episodeListData.size());
        fmt::print(fmt::fg(fmt::color::lime_green), "OK!\n");
        return episodeListData;
    }

    void Animepahe::extractor(
        bool isSeries,
        const std::string &link,
        bool isAllEpisodes,
        const std::vector<int> &episodes,
        bool exportLinks,
        bool createZip
    )
    {
        /* print config */
        fmt::print("\n * exportLinks: ");
        exportLinks ? fmt::print(fmt::fg(fmt::color::cyan), "true\n") : fmt::print("false\n");
        fmt::print(" * createZip: ", createZip);
        createZip ? fmt::print(fmt::fg(fmt::color::cyan), "true\n") : fmt::print("false\n");
        /* Request Metadata */
        extract_link_metadata(link, isSeries);

        /* Extract Links */
        const std::vector<std::map<std::string, std::string>> epData = extract_link_content(link, episodes, isSeries, isAllEpisodes);
        if (isSeries)
        {
            fmt::print(" * Episodes Range: {}\n", isAllEpisodes ? "All" : vectorToString(episodes));
        }
        
        const char* CLEAR_LINE = "\033[2K";   // Clear entire line
        const char* MOVE_UP = "\033[1A";      // Move cursor up 1 line
        const char* CURSOR_START = "\r";      // Return to start of line

        /* Extract Kwik from pahe.win */
        KwikPahe kwikpahe;
        std::vector<std::string> directLinks;

        for (int i = 0; i < epData.size(); ++i)
        {
            fmt::print("\n\r * Processing :");
            fmt::print(fmt::fg(fmt::color::cyan), fmt::format(" EP{}", padIntWithZero(i + 1)));
            std::string link = kwikpahe.extract_kwik_link(epData[i].at("dPaheLink"));
            directLinks.push_back(link);
            for (int i = 0; i < 3; ++i) {
                fmt::print("{}{}{}", MOVE_UP, CLEAR_LINE, CURSOR_START);
            }
            fmt::print("\r * Processing : EP{}", padIntWithZero(i + 1));
            fmt::print(fmt::fg(fmt::color::lime_green), " OK!");
        }

        if (exportLinks)
        {
            std::ofstream exportfile("links.txt");
            if (exportfile.is_open())
            {
                for (auto &link: directLinks)
                {
                    exportfile << link << "\n";
                }
                exportfile.close();
            }
            fmt::print("\n\n * Exported : links.txt\n");
        }
        else
        {
            fmt::print("\n\n * createZip: {}\n", createZip);
        }

    }
}
