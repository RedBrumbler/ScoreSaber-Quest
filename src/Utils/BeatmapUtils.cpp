#include "Utils/BeatmapUtils.hpp"
#include "GlobalNamespace/BeatmapDataItem.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
// #include "System::Threading::Tasks::Task_1.hpp"
// System::Collections::Generic::LinkedListNode_1

#include "GlobalNamespace/BeatmapEnvironmentHelper.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/EnvironmentsListSO.hpp"
#include "GlobalNamespace/IBeatmapDataBasicInfo.hpp"
#include "GlobalNamespace/SliderData.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
//
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "UnityEngine/Resources.hpp"
#include "logging.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"
#include <tuple>

using namespace GlobalNamespace;

namespace BeatmapUtils
{

    GlobalNamespace::PlayerDataModel* playerDataModel;
    std::vector<int> routines;

    int getDiff(GlobalNamespace::IDifficultyBeatmap* beatmap)
    {
        return beatmap->get_difficultyRank();
    }

    custom_types::Helpers::Coroutine getMaxScoreCoroutine(GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap, std::function<void(int maxScore)> callback)
    {
        if (playerDataModel == nullptr)
        {
            playerDataModel = UnityEngine::Object::FindObjectOfType<GlobalNamespace::PlayerDataModel*>();
        }
        int crIndex = routines.size() + 1;
        routines.push_back(crIndex);
        auto* envInfo = GlobalNamespace::BeatmapEnvironmentHelper::GetEnvironmentInfo(difficultyBeatmap);
        auto* result = difficultyBeatmap->GetBeatmapDataAsync(envInfo, playerDataModel->playerData->playerSpecificSettings);
        while (!result->get_IsCompleted())
            co_yield nullptr;
        auto* data = result->get_ResultOnSuccess();
        if (routines.empty() || routines.size() != crIndex)
            co_return;
        ClearVector<int>(&routines);
        auto beatmapDataBasicInfo = il2cpp_utils::try_cast<GlobalNamespace::IBeatmapDataBasicInfo>(data).value_or(nullptr);
        int blockCount = beatmapDataBasicInfo->get_cuttableNotesCount();
        callback(getMaxScoreFromCuttableNotesCount(blockCount));
        co_return;
    }

    int getMaxScore(GlobalNamespace::IBeatmapDataBasicInfo* beatmapDataBasicInfo)
    {
        int blockCount = beatmapDataBasicInfo->get_cuttableNotesCount();
        return getMaxScoreFromCuttableNotesCount(blockCount);
    }

    std::tuple<GlobalNamespace::IBeatmapDataBasicInfo*, GlobalNamespace::IReadonlyBeatmapData*> getBeatmapData(GlobalNamespace::IDifficultyBeatmap* beatmap)
    {
        if (playerDataModel == nullptr)
        {
            playerDataModel = UnityEngine::Object::FindObjectOfType<GlobalNamespace::PlayerDataModel*>();
        }
        auto environmentInfo = GlobalNamespace::BeatmapEnvironmentHelper::GetEnvironmentInfo(beatmap);
        System::Threading::Tasks::Task_1<GlobalNamespace::IReadonlyBeatmapData*>* result = nullptr;
        QuestUI::MainThreadScheduler::Schedule([&]() {
            result = beatmap->GetBeatmapDataAsync(environmentInfo, playerDataModel->playerData->playerSpecificSettings);
        });
        while (!result || !result->get_IsCompleted())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        auto data = result->get_ResultOnSuccess();
        auto beatmapDataBasicInfo = il2cpp_utils::try_cast<GlobalNamespace::IBeatmapDataBasicInfo>(data).value_or(nullptr);
        return std::make_tuple(beatmapDataBasicInfo, data);
    }

    bool containsV3Stuff(GlobalNamespace::IReadonlyBeatmapData* beatmapData)
    {
        // iterate over allBeatmapDataItems

        auto list = beatmapData->get_allBeatmapDataItems();
        for (auto i = list->head; i->next != list->head; i = i->next)
        {
            if (i->item->type == 0 && il2cpp_utils::try_cast<GlobalNamespace::SliderData>(i->item).value_or(nullptr))
            {
                return true;
            }
        }
        return false;
    }

    int getMaxScoreFromCuttableNotesCount(int cuttableNotesCount)
    {
        int maxScore;
        if (cuttableNotesCount < 14)
        {
            if (cuttableNotesCount == 1)
            {
                maxScore = 115;
            }
            else if (cuttableNotesCount < 5)
            {
                maxScore = (cuttableNotesCount - 1) * 230 + 115;
            }
            else
            {
                maxScore = (cuttableNotesCount - 5) * 460 + 1035;
            }
        }
        else
        {
            maxScore = (cuttableNotesCount - 13) * 920 + 4715;
        }
        if (maxScore == 0)
            return -1;
        return maxScore;
    }

    int OldMaxRawScoreForNumberOfNotes(int noteCount)
    {
        int num = 0;
        int num2 = 1;
        while (num2 < 8)
        {
            if (noteCount >= num2 * 2)
            {
                num += num2 * num2 * 2 + num2;
                noteCount -= num2 * 2;
                num2 *= 2;
                continue;
            }
            num += num2 * noteCount;
            noteCount = 0;
            break;
        }
        num += noteCount * num2;
        return num * 115;
    }

    GlobalNamespace::GameplayModifiers* GetModifiersFromStrings(std::vector<std::string> modifiers)
    {
        auto energyType = GameplayModifiers::EnergyType::Bar;
        auto obstacleType = GameplayModifiers::EnabledObstacleType::All;
        auto songSpeed = GameplayModifiers::SongSpeed::Normal;

        bool NF = false;
        bool IF = false;
        bool NB = false;
        bool DA = false;
        bool GN = false;
        bool NA = false;
        bool PM = false;
        bool SC = false;
        bool SA = false;

        // iterate modifiers
        for (auto& modifier : modifiers)
        {
            if (modifier == "BE")
            {
                energyType = GameplayModifiers::EnergyType::Battery;
            }
            else if (modifier == "NF")
            {
                NF = true;
            }
            else if (modifier == "IF")
            {
                IF = true;
            }
            else if (modifier == "NO")
            {
                obstacleType = GameplayModifiers::EnabledObstacleType::NoObstacles;
            }
            else if (modifier == "NB")
            {
                NB = true;
            }
            else if (modifier == "DA")
            {
                DA = true;
            }
            else if (modifier == "GN")
            {
                GN = true;
            }
            else if (modifier == "NA")
            {
                NA = true;
            }
            else if (modifier == "SS")
            {
                songSpeed = GameplayModifiers::SongSpeed::Slower;
            }
            else if (modifier == "FS")
            {
                songSpeed = GameplayModifiers::SongSpeed::Faster;
            }
            else if (modifier == "SF")
            {
                songSpeed = GameplayModifiers::SongSpeed::SuperFast;
            }
            else if (modifier == "PM")
            {
                PM = true;
            }
            else if (modifier == "SC")
            {
                SC = true;
            }
            else if (modifier == "SA")
            {
                SA = true;
            }
        }

        return GameplayModifiers::New_ctor(energyType, NF, IF, false, obstacleType, NB, false, SA, DA, songSpeed, NA, GN, PM, false, SC);
    }

} // namespace BeatmapUtils