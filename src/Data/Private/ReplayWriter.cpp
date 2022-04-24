#include "Data/Private/ReplayWriter.hpp"

extern "C"
{
#include "Utils/lzma/simple.h"
}

#include <codecvt>
#include <locale>
#include <stdlib.h>
#include <string>

#include "Services/PlayerService.hpp"

#include "logging.hpp"
#include "static.hpp"

using namespace std;

namespace ScoreSaber::Data::Private::ReplayWriter
{
    int _pointerSize = 38;
    void Write(ReplayFile* file)
    {
        std::string tmpFilePath = ScoreSaber::Static::REPLAY_TMP_DIR + "/b2dbeb695fa205804b1e5e72650ad2bb.tmp";

        // Open tmp replay file
        ofstream outputStream = ofstream(tmpFilePath, ios::binary);
        std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
        outputStream.imbue(utf8_locale);

        int pointerLocation = outputStream.tellp();
        for (int i = 0; i < _pointerSize; i += 4)
        {
            WriteInt(0, outputStream);
        }

        // Write Data
        int metadataPointer = outputStream.tellp();
        WriteMetadata(file->metadata, outputStream);
        int poseKeyframePointer = outputStream.tellp();
        WriteVRPoseGroupList(file->poseKeyframes, outputStream);
        int heightEventsPointer = outputStream.tellp();
        WriteHeightChangeList(file->heightKeyframes, outputStream);
        int noteEventsPointer = outputStream.tellp();
        WriteNoteEventList(file->noteKeyframes, outputStream);
        int scoreEventsPointer = outputStream.tellp();
        WriteScoreEventList(file->scoreKeyframes, outputStream);
        int comboEventsPointer = outputStream.tellp();
        WriteComboEventList(file->comboKeyframes, outputStream);
        int multiplierEventsPointer = outputStream.tellp();
        WriteMultiplierEventList(file->multiplierKeyframes, outputStream);
        int energyEventsPointer = outputStream.tellp();
        WriteEnergyEventList(file->energyKeyframes, outputStream);

        // Write pointers
        outputStream.seekp(pointerLocation);
        WriteInt(metadataPointer, outputStream);
        WriteInt(poseKeyframePointer, outputStream);
        WriteInt(heightEventsPointer, outputStream);
        WriteInt(noteEventsPointer, outputStream);
        WriteInt(scoreEventsPointer, outputStream);
        WriteInt(comboEventsPointer, outputStream);
        WriteInt(multiplierEventsPointer, outputStream);
        WriteInt(energyEventsPointer, outputStream);

        outputStream.flush();
        outputStream.close();

        // Read tmp replay file into memory
        ifstream readStream(tmpFilePath, ios_base::in | ios::binary);
        vector<unsigned char> uncompressedReplayBytes;
        unsigned char currentChar = readStream.get();
        while (readStream.good())
        {
            uncompressedReplayBytes.push_back(currentChar);
            currentChar = readStream.get();
        }

        size_t uncompressedSize = uncompressedReplayBytes.size();
        unsigned char* uncompressed = uncompressedReplayBytes.data();

        // Delete tmp replay file
        unlink(tmpFilePath.c_str());

        // Compress replay file
        unsigned char* compressed;
        size_t sz;
        int result = simpleCompress(ELZMA_lzma, uncompressed, uncompressedSize, &compressed, &sz);
        if (result == ELZMA_E_OK)
        {
            // If compression went okay, add file header to compressed replay and write to disk
            ofstream finalOutputStream = ofstream(ScoreSaber::Static::REPLAY_DIR + "/76561198283584459-Chug Jug With You-Expert-Standard-4D5D4F9D86C8FD56610D0D157E6EAABFABA9B1C9.dat", ios::binary);
            std::locale utf8_locale(std::locale(), new codecvt_utf8<char16_t>);
            finalOutputStream.imbue(utf8_locale);
            WriteRawString("ScoreSaber Replay 👌🤠\r\n", finalOutputStream);
            finalOutputStream.write(reinterpret_cast<char*>(compressed), sz);
            finalOutputStream.flush();
            finalOutputStream.close();
        }
    }

    int WriteMetadata(Metadata* metadata, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteString(metadata->Version, outputStream);
        bytesWritten += WriteString(metadata->LevelID, outputStream);
        bytesWritten += WriteInt(metadata->Difficulty, outputStream);
        bytesWritten += WriteString(metadata->Characteristic, outputStream);
        bytesWritten += WriteString(metadata->Environment, outputStream);
        bytesWritten += WriteStringArray(metadata->Modifiers, outputStream);
        bytesWritten += WriteFloat(metadata->NoteSpawnOffset, outputStream);
        bytesWritten += WriteBool(metadata->LeftHanded, outputStream);
        bytesWritten += WriteFloat(metadata->InitialHeight, outputStream);
        bytesWritten += WriteFloat(metadata->RoomRotation, outputStream);
        bytesWritten += WriteVRPosition(metadata->RoomCenter, outputStream);
        bytesWritten += WriteFloat(metadata->FailTime, outputStream);
        return bytesWritten;
    }

    int WriteVRPoseGroup(VRPoseGroup vrPoseGroup, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteVRPose(vrPoseGroup.Head, outputStream);
        bytesWritten += WriteVRPose(vrPoseGroup.Left, outputStream);
        bytesWritten += WriteVRPose(vrPoseGroup.Right, outputStream);
        bytesWritten += WriteInt(vrPoseGroup.FPS, outputStream);
        bytesWritten += WriteFloat(vrPoseGroup.Time, outputStream);
        return bytesWritten;
    }

    int WriteVRPose(VRPose vrPose, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteVRPosition(vrPose.Position, outputStream);
        bytesWritten += WriteVRRotation(vrPose.Rotation, outputStream);
        return bytesWritten;
    }

    int WriteHeightEvent(HeightEvent heightEvent, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteFloat(heightEvent.Height, outputStream);
        bytesWritten += WriteFloat(heightEvent.Time, outputStream);
        return bytesWritten;
    }

    int WriteNoteEvent(NoteEvent noteEvent, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteNoteID(noteEvent.TheNoteID, outputStream);
        bytesWritten += WriteInt((int)noteEvent.EventType, outputStream);
        bytesWritten += WriteVRPosition(noteEvent.CutPoint, outputStream);
        bytesWritten += WriteVRPosition(noteEvent.CutNormal, outputStream);
        bytesWritten += WriteVRPosition(noteEvent.SaberDirection, outputStream);
        bytesWritten += WriteInt(noteEvent.SaberType, outputStream);
        bytesWritten += WriteBool(noteEvent.DirectionOK, outputStream);
        bytesWritten += WriteFloat(noteEvent.SaberSpeed, outputStream);
        bytesWritten += WriteFloat(noteEvent.CutAngle, outputStream);
        bytesWritten += WriteFloat(noteEvent.CutDistanceToCenter, outputStream);
        bytesWritten += WriteFloat(noteEvent.CutDirectionDeviation, outputStream);
        bytesWritten += WriteFloat(noteEvent.BeforeCutRating, outputStream);
        bytesWritten += WriteFloat(noteEvent.AfterCutRating, outputStream);
        bytesWritten += WriteFloat(noteEvent.Time, outputStream);
        bytesWritten += WriteFloat(noteEvent.UnityTimescale, outputStream);
        bytesWritten += WriteFloat(noteEvent.TimeSyncTimescale, outputStream);
        return bytesWritten;
    }

    int WriteNoteID(NoteID noteID, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteFloat(noteID.Time, outputStream);
        bytesWritten += WriteInt(noteID.LineLayer, outputStream);
        bytesWritten += WriteInt(noteID.LineIndex, outputStream);
        bytesWritten += WriteInt(noteID.ColorType, outputStream);
        bytesWritten += WriteInt(noteID.CutDirection, outputStream);
        return bytesWritten;
    }

    int WriteScoreEvent(ScoreEvent scoreEvent, ofstream& outputStream)
    {

        int bytesWritten = 0;
        bytesWritten += WriteInt(scoreEvent.Score, outputStream);
        bytesWritten += WriteFloat(scoreEvent.Time, outputStream);
        return bytesWritten;
    }

    int WriteComboEvent(ComboEvent scoreEvent, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(scoreEvent.Combo, outputStream);
        bytesWritten += WriteFloat(scoreEvent.Time, outputStream);
        return bytesWritten;
    }

    int WriteMultiplierEvent(MultiplierEvent multiplierEvent, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(multiplierEvent.Multiplier, outputStream);
        bytesWritten += WriteFloat(multiplierEvent.NextMultiplierProgress, outputStream);
        bytesWritten += WriteFloat(multiplierEvent.Time, outputStream);
        return bytesWritten;
    }

    int WriteEnergyEvent(EnergyEvent energyEvent, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteFloat(energyEvent.Energy, outputStream);
        bytesWritten += WriteFloat(energyEvent.Time, outputStream);
        return bytesWritten;
    }

    // Lists
    int WriteStringArray(vector<string> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteString(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteVRPoseGroupList(vector<VRPoseGroup> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteVRPoseGroup(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteHeightChangeList(vector<HeightEvent> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteHeightEvent(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteNoteEventList(vector<NoteEvent> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteNoteEvent(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteScoreEventList(vector<ScoreEvent> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteScoreEvent(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteComboEventList(vector<ComboEvent> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteComboEvent(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteMultiplierEventList(vector<MultiplierEvent> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteMultiplierEvent(values[i], outputStream);
        }
        return bytesWritten;
    }

    int WriteEnergyEventList(vector<EnergyEvent> values, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteInt(values.size(), outputStream);
        for (int i = 0; i < values.size(); i++)
        {
            bytesWritten += WriteEnergyEvent(values[i], outputStream);
        }
        return bytesWritten;
    }

    // Primitives
    int WriteString(string value, ofstream& outputStream)
    {
        int bytesWritten = 0;
        const char* cString = value.c_str();
        size_t stringLength = strlen(cString);

        bytesWritten += WriteInt((int)stringLength, outputStream);
        outputStream.write(cString, stringLength);
        bytesWritten += stringLength;
        return bytesWritten;
    }

    int WriteRawString(string value, ofstream& outputStream)
    {
        int bytesWritten = 0;
        const char* cString = value.c_str();
        size_t stringLength = strlen(cString);

        outputStream.write(cString, stringLength);
        bytesWritten += stringLength;
        return bytesWritten;
    }

    int WriteInt(int value, ofstream& outputStream)
    {
        outputStream.write(reinterpret_cast<const char*>(&value), sizeof(int));
        return 4;
    }

    int WriteFloat(float value, ofstream& outputStream)
    {
        outputStream.write(reinterpret_cast<const char*>(&value), sizeof(float));
        return 4;
    }

    int WriteBool(float value, ofstream& outputStream)
    {
        outputStream.write(reinterpret_cast<const char*>(&value), sizeof(bool));
        return 1;
    }

    int WriteVRPosition(VRPosition position, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteFloat(position.X, outputStream);
        bytesWritten += WriteFloat(position.Y, outputStream);
        bytesWritten += WriteFloat(position.Z, outputStream);
        return bytesWritten;
    }

    int WriteVRRotation(VRRotation rotation, ofstream& outputStream)
    {
        int bytesWritten = 0;
        bytesWritten += WriteFloat(rotation.X, outputStream);
        bytesWritten += WriteFloat(rotation.Y, outputStream);
        bytesWritten += WriteFloat(rotation.Z, outputStream);
        bytesWritten += WriteFloat(rotation.W, outputStream);
        return bytesWritten;
    }

} // namespace ScoreSaber::Data::Private::ReplayWriter
