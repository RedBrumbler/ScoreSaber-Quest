#include "Services/PlayerService.hpp"
#include "Utils/WebUtils.hpp"

#include "Data/Private/AuthResponse.hpp"
#include "System/IO/Directory.hpp"
#include "Utils/StringUtils.hpp"
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"
#include "logging.hpp"
#include "questui/shared/BeatSaberUI.hpp"

#define STEAM_KEY_PATH "/sdcard/ModData/Mods/ScoreSaber/scoresaber_DO_NOT_SHARE.scary"
#define SCORESABER_DATA_PATH "/sdcard/ModData/Mods/ScoreSaber"

using namespace StringUtils;

namespace ScoreSaber::Services::PlayerService
{
    playerInfo_t playerInfo;
    void AuthenticateUser(std::function<void(LoginStatus)> finished)
    {
        if (!direxists(SCORESABER_DATA_PATH))
        {
            System::IO::Directory::CreateDirectory(il2cpp_utils::newcsstr(SCORESABER_DATA_PATH));
        }

        std::string steamKey = "fb6580ef414bf07";
        std::string playerId = "76561198283584459";

        if (fileexists(STEAM_KEY_PATH))
        {
            std::string rawSteamThing = readfile(STEAM_KEY_PATH);
            std::vector<std::string> splitRawSteamThing = split(rawSteamThing, ':');

            steamKey = splitRawSteamThing[0];
            playerId = splitRawSteamThing[1];

            std::string steamKey = readfile(STEAM_KEY_PATH);
        }

        // UMBY: Check if steam key is null (for release)

        // UMBY: Obfuscate auth url
        // UMBY: Friends

        std::string postData = "at=2&playerId=" + playerId + "&nonce=" + steamKey + "&friends=3692740027462863,76561198064659288,76561198283584459,76561198278902434,76561198353781972,76561199210789241&name=nah";

        std::string url = "http://192.168.1.8:9999/api/game/auth";
        // std::string url = "https://scoresaber.com/api/game/auth";

        WebUtils::PostAsync(url, postData, 6000, [=](long code, std::string result) {
            if (code == 200)
            {
                rapidjson::Document jsonDocument;
                jsonDocument.Parse(result.data());
                ScoreSaber::Data::Private::AuthResponse authResponse(jsonDocument.GetObject());
                playerInfo.playerKey = authResponse.a;
                playerInfo.serverKey = authResponse.e;

                GetPlayerInfo(playerId, true, [=](std::optional<Data::Player> playerData) {
                    if (playerData.has_value())
                    {
                        playerInfo.localPlayerData = playerData.value();
                    }
                    else
                    {
                        playerInfo.localPlayerData = Data::Player(playerId);
                        INFO("Failed to get players info");
                    }
                    playerInfo.loginStatus = LoginStatus::Success;
                    finished(LoginStatus::Success);
                });
            }
            else
            {
                INFO("Authentication error");
                playerInfo.loginStatus = LoginStatus::Error;
                finished(LoginStatus::Error);
            }
        });
    }

    void GetPlayerInfo(std::string playerId, bool full, std::function<void(std::optional<Data::Player>)> finished)
    {

        std::string url = string_format("http://192.168.1.8:4000/api/player/%s", playerId.c_str());

        // std::string url = string_format("https://scoresaber.com/api/player/%s", playerId.c_str());

        if (full)
        {
            url = string_format("%s/full", url.c_str());
        }
        else
        {
            url = string_format("%s/basic", url.c_str());
        }

        WebUtils::GetAsync(url, [&, finished](long code, std::string result) {
            if (code == 200)
            {
                try
                {
                    rapidjson::Document jsonDocument;
                    jsonDocument.Parse(result.data());
                    Data::Player playerResponse(jsonDocument.GetObject());
                    finished(std::make_optional<Data::Player>(playerResponse));
                }
                catch (const std::exception& e)
                {
                    finished(std::nullopt);
                }
            }
            else
            {
                finished(std::nullopt);
            }
        });
    }

} // namespace ScoreSaber::Services::PlayerService
