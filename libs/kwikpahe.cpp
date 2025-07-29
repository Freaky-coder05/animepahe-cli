#include <kwikpahe.hpp>
#include <utils.hpp>
#include <cpr/cpr.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <re2/re2.h>
#include <string>
/* DECODER LIBS */
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <cctype>

namespace AnimepaheCLI
{
    std::string encodedString = "XXX"; // Encoded string
    int zp = 17;                       // Not used in decoding
    std::string alphabetKey = "XXX";   // Alphabet key
    int offset = 1;                    // Offset
    int base = 5;                      // Base
    int placeholder = 24;              // Placeholder (unused)

    std::string baseAlphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+/";

    int KwikPahe::_0xe16c(const std::string &IS, int Iy, int ms)
    {
        std::string h = baseAlphabet.substr(0, Iy);
        std::string i = baseAlphabet.substr(0, ms);

        // Decode string IS from base Iy to int j
        int j = 0;
        for (int idx = 0; idx < IS.size(); ++idx)
        {
            char ch = IS[IS.size() - 1 - idx]; // reverse order
            size_t pos = h.find(ch);
            if (pos != std::string::npos)
            {
                j += static_cast<int>(pos) * static_cast<int>(std::pow(Iy, idx));
            }
        }

        // Convert int j to base ms string
        if (j == 0)
            return i[0];

        std::string k;
        while (j > 0)
        {
            k = i[j % ms] + k;
            j /= ms;
        }

        return std::stoi(k);
    }

    std::string KwikPahe::decodeJSStyle(const std::string &Hb, int zp, const std::string &Wg, int Of, int Jg, int gj_placeholder)
    {
        std::string gj;

        for (size_t i = 0; i < Hb.size(); ++i)
        {
            std::string s;
            while (Hb[i] != Wg[Jg])
            {
                s += Hb[i];
                i++;
                if (i >= Hb.size())
                    break;
            }

            for (size_t j = 0; j < Wg.size(); ++j)
            {
                std::string from(1, Wg[j]);
                std::string to = std::to_string(j);
                size_t pos;
                while ((pos = s.find(from)) != std::string::npos)
                {
                    s.replace(pos, 1, to);
                }
            }

            int code = _0xe16c(s, Jg, 10) - Of;
            gj += static_cast<char>(code);
        }

        return gj;
    }

    std::string KwikPahe::fetch_kwik_direct(const std::string &kwikLink, const std::string &token, const std::string &kwik_session)
    {
        // Set up cookies
        cpr::Header headers = cpr::Header{
            {"referer", kwikLink},
            {"cookie", "kwik_session=" + kwik_session},
        };
        // Set up form data
        cpr::Payload data = cpr::Payload{{"_token", token}};

        // Make POST request with redirects disabled
        cpr::Response response = cpr::Post(
            cpr::Url{kwikLink},
            headers,
            data,
            cpr::Redirect(false),
            cpr::HttpVersion{cpr::HttpVersionCode::VERSION_1_1}
        );

        // Check if status code is 302 (redirect)
        if (response.status_code == 302)
        {
            // Extract the redirect location from the response header
            std::string redirectLocation;
            re2::StringPiece rawHeader(response.raw_header);
            if (RE2::FindAndConsume(&rawHeader, R"re(Location:\s*(https?://\S+))re", &redirectLocation))
            {
                return redirectLocation;
            }
            throw std::runtime_error(fmt::format("Redirect Location not found in response from {}", kwikLink));
        }
        else
        {
            throw std::runtime_error(fmt::format("Redirect Location not found in response from {}", kwikLink));
        }
    }

    std::string KwikPahe::fetch_kwik_dlink(const std::string &kwikLink, int retries)
    {
        if (retries <= 0)
        {
            throw std::runtime_error("Kwik fetch failed: exceeded retry limit");
        }

        cpr::Response response = cpr::Get(cpr::Url{kwikLink});
        if (response.status_code != 200)
        {
            throw std::runtime_error(fmt::format("Failed to Get Kwik from {}, StatusCode: {}", kwikLink, response.status_code));
        }

        RE2::GlobalReplace(&response.text, R"((\r\n|\r|\n))", "");
        re2::StringPiece ENCODE_CONSUME = response.text;
        re2::StringPiece SESSION_CONSUME = response.raw_header;

        std::string kwik_session;
        std::string link, token;
        std::string directLink;

        RE2::FindAndConsume(
            &ENCODE_CONSUME,
            R"re(\(\s*"([^",]*)"\s*,\s*\d+\s*,\s*"([^",]*)"\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*\d+[a-zA-Z]?\s*\))re",
            &encodedString, &alphabetKey, &offset, &base
        );

        re2::StringPiece decoded = decodeJSStyle(encodedString, zp, alphabetKey, offset, base, placeholder);
        // Step 1: extract the outer tag content
        RE2::FindAndConsume(&decoded, R"re("(https?://kwik\.[^/\s"]+/[^/\s"]+/[^"\s]*)")re", &link);
        // Step 2: look for the token separately
        RE2::FindAndConsume(&decoded, R"re(name="_token"[^"]*"(\S*)">)re", &token);
        /* kwik session */
        RE2::FindAndConsume(&SESSION_CONSUME, R"re(kwik_session=([^;]*);)re", &kwik_session);

        if (token.empty() || link.empty())
        {
            return fetch_kwik_dlink(kwikLink, retries - 1);
        }
        else
        {
            directLink = fetch_kwik_direct(link, token, kwik_session);
        }

        return directLink;
    }

    std::string KwikPahe::extract_kwik_link(const std::string &link)
    {
        fmt::print("\n\r * Extracting Kwik Link...");
        cpr::Response response = cpr::Get(cpr::Url{link});
        if (response.status_code != 200)
        {
            throw std::runtime_error(fmt::format("Failed to Get Kwik from {}, StatusCode: {}", link, response.status_code));
        }
        
        RE2::GlobalReplace(&response.text, R"((\r\n|\r|\n))", "");
        std::string cleanText = sanitize_utf8(response.text);
        re2::StringPiece NORMAL_CONSUME = cleanText;
        re2::StringPiece ENCODE_CONSUME = cleanText;
        std::string kwikLink;
        
        RE2::FindAndConsume(&NORMAL_CONSUME, R"re("(https?://kwik\.[^/\s"]+/[^/\s"]+/[^"\s]*)")re", &kwikLink);
        if (kwikLink.empty())
        {
            RE2::FindAndConsume(
                &ENCODE_CONSUME,
                R"re(\(\s*"([^",]*)"\s*,\s*\d+\s*,\s*"([^",]*)"\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*\d+[a-zA-Z]?\s*\))re",
                &encodedString, &alphabetKey, &offset, &base
            );

            std::string decodedString = decodeJSStyle(encodedString, zp, alphabetKey, offset, base, placeholder);
            re2::StringPiece decoded(decodedString);
            RE2::FindAndConsume(&decoded, R"re("(https?://kwik\.[^/\s"]+/[^/\s"]+/[^"\s]*)")re", &kwikLink);
            RE2::Replace(&kwikLink, R"re((https:\/\/kwik\.[^\/]+\/)d\/)re", "\\1f/");
        }

        if (kwikLink.empty())
        {
            throw std::runtime_error(fmt::format("Failed to extract Kwik link from {}", link));
        }
        
        fmt::print("\r * Extracting Kwik Link :");
        fmt::print(fmt::fg(fmt::color::lime_green), " OK!\n");
        fmt::print(" * Fetching Kwik Direct Link...");
        std::string directLink = fetch_kwik_dlink(kwikLink);
        fmt::print("\r * Fetching Kwik Direct Link :");
        fmt::print(fmt::fg(fmt::color::lime_green), " OK!\n");
        return directLink;
    }
}
