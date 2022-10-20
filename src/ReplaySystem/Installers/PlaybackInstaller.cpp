#include "ReplaySystem/Installers/PlaybackInstaller.hpp"

#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "ReplaySystem/Playback/HeightPlayer.hpp"
#include "ReplaySystem/Playback/NotePlayer.hpp"
#include "ReplaySystem/Playback/PosePlayer.hpp"
#include "ReplaySystem/Playback/ReplayTimeSyncController.hpp"
#include "ReplaySystem/Playback/ScorePlayer.hpp"
#include "Zenject/ConcreteBinderGeneric_1.hpp"
#include "Zenject/DiContainer.hpp"
#include "Zenject/FromBinderNonGeneric.hpp"
#include "lapiz/shared/utilities/ZenjectExtensions.hpp"
#include "logging.hpp"

DEFINE_TYPE(ScoreSaber::ReplaySystem::Installers, PlaybackInstaller);

using namespace Lapiz::Zenject::ZenjectExtensions;

namespace ScoreSaber::ReplaySystem::Installers
{
    void PlaybackInstaller::ctor(GlobalNamespace::GameplayCoreSceneSetupData* gameplayCoreSceneSetupData)
    {
        _gameplayCoreSceneSetupData = gameplayCoreSceneSetupData;
    }

    void PlaybackInstaller::InstallBindings()
    {
        auto container = get_Container();
        container->BindInterfacesAndSelfTo<Playback::PosePlayer*>()->AsSingle();
        container->BindInterfacesTo<Playback::NotePlayer*>()->AsSingle();
        container->BindInterfacesTo<Playback::ScorePlayer*>()->AsSingle();
        if (_gameplayCoreSceneSetupData->playerSpecificSettings->automaticPlayerHeight)
        {
            container->BindInterfacesTo<Playback::HeightPlayer*>()->AsSingle();
        }

        // container->BindInterfacesAndSelfTo<Playback::ReplayTimeSyncController*>()->AsSingle();
    }
} // namespace ScoreSaber::ReplaySystem::Installers
