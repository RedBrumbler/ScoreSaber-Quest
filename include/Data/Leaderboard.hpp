#pragma once
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "Data/LeaderboardInfo.hpp"
#include "Data/Score.hpp"
#include <vector>

struct Il2CppString;

namespace ScoreSaber::Data
{
    struct Leaderboard
    {
        Leaderboard();
        Leaderboard(Il2CppString* string);
        Leaderboard(std::string_view string);
        Leaderboard(std::u16string_view string);
        Leaderboard(const rapidjson::Document&& doc);
        Leaderboard(const rapidjson::GenericDocument<rapidjson::UTF16<char16_t>>&& doc);
        Leaderboard(rapidjson::GenericObject<true, rapidjson::Value> doc);
        Leaderboard(rapidjson::GenericObject<true, rapidjson::GenericValue<rapidjson::UTF16<char16_t>>> doc);
        Leaderboard(rapidjson::GenericObject<false, rapidjson::Value> doc);
        Leaderboard(rapidjson::GenericObject<false, rapidjson::GenericValue<rapidjson::UTF16<char16_t>>> doc);

        LeaderboardInfo leaderboardInfo;
        std::vector<Score> scores;
    };
}