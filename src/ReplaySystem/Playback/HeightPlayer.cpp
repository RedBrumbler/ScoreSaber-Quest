#include "ReplaySystem/Playback/HeightPlayer.hpp"
#include "ReplaySystem/ReplayLoader.hpp"
#include "System/Action_1.hpp"
#include "UnityEngine/Mathf.hpp"
#include "UnityEngine/Transform.hpp"
#include "logging.hpp"
#include <algorithm>

using namespace UnityEngine;
using namespace ScoreSaber::Data::Private;

DEFINE_TYPE(ScoreSaber::ReplaySystem::Playback, HeightPlayer);

namespace ScoreSaber::ReplaySystem::Playback
{
    void HeightPlayer::ctor(GlobalNamespace::AudioTimeSyncController* audioTimeSyncController, GlobalNamespace::PlayerHeightDetector* playerHeightDetector)
    {
        _audioTimeSyncController = audioTimeSyncController;
        _playerHeightDetector = playerHeightDetector;
        _sortedHeightEvents = ReplayLoader::LoadedReplay->heightKeyframes;
    }

    void HeightPlayer::Initialize()
    {
        _playerHeightDetector->OnDestroy();
    }

    void HeightPlayer::Tick()
    {
        if (_lastIndex >= _sortedHeightEvents.size() - 1)
        {
            return;
        }

        HeightEvent activeEvent = _sortedHeightEvents[_lastIndex];
        if (_audioTimeSyncController->get_songEndTime() >= activeEvent.Time)
        {
            _lastIndex++;
            if (_playerHeightDetector->playerHeightDidChangeEvent != nullptr)
            {
                _playerHeightDetector->playerHeightDidChangeEvent->Invoke(activeEvent.Height);
            }
        }
    }

    void HeightPlayer::TimeUpdate(float songTime)
    {
        for (int c = 0; c < _sortedHeightEvents.size(); c++)
        {
            if (_sortedHeightEvents[c].Time >= songTime)
            {
                _lastIndex = c;
                Tick();
                break;
            }
        }
        if (_playerHeightDetector->playerHeightDidChangeEvent != nullptr)
        {
            _playerHeightDetector->playerHeightDidChangeEvent->Invoke(_sortedHeightEvents[_sortedHeightEvents.size() - 1].Height);
        }
    }
} // namespace ScoreSaber::ReplaySystem::Playback