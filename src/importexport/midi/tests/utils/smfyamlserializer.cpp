/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "smfyamlserializer.h"

#include <cstdint>
#include <stack>
#include <string_view>
#include <vector>

#include <QFile>

#include "global/log.h"
#include "global/serialization/textstream.h"

#include "importexport/midi/internal/midishared/midievent.h"
#include "importexport/midi/internal/midishared/midifile.h"

using namespace std::literals;
using namespace muse;

namespace mu::iex::midi {
namespace {
// very basic yaml writer
class YamlStreamWriter
{
public:
    explicit YamlStreamWriter(io::IODevice* out)
        : m_textOut(out)
    {
    }

    void flush()
    {
        m_textOut.flush();
    }

    void beginMapping()
    {
        pushNode(ComplexNodeType::Mapping);
    }

    void endMapping()
    {
        popNode(ComplexNodeType::Mapping);
    }

    template<typename Scalar>
    void mapToScalar(const std::string_view key, Scalar&& value)
    {
        IF_ASSERT_FAILED(isTopNode(ComplexNodeType::Mapping)) {
            return;
        }

        writeIndent();
        m_textOut << key << ": " << std::forward<Scalar>(value) << '\n';
    }

    void mapToScalar(const std::string_view key, const bool value)
    {
        writeKeyValue(key, value ? "true" : "false");
    }

    void mapToScalar(const std::string_view key, const int value)
    {
        writeKeyValue(key, value);
    }

    void mapToScalar(const std::string_view key, const std::string_view value)
    {
        writeKeyValue(key, value);
    }

    void mapToMapping(const std::string_view key)
    {
        IF_ASSERT_FAILED(isTopNode(ComplexNodeType::Mapping)) {
            return;
        }

        writeIndent();
        m_textOut << key << ":\n";
        beginMapping();
    }

    void mapToSequence(const std::string_view key)
    {
        IF_ASSERT_FAILED(isTopNode(ComplexNodeType::Mapping)) {
            return;
        }

        writeIndent();
        m_textOut << key << ":\n";
        beginSequence();
    }

    void beginSequence()
    {
        pushNode(ComplexNodeType::Sequence);
    }

    void endSequence()
    {
        popNode(ComplexNodeType::Sequence);
    }

    void sequenceAddMapping()
    {
        writeIndent();
        m_textOut << "-\n";
        beginMapping();
    }

    void sequenceAddSequence()
    {
        writeIndent();
        m_textOut << "-\n";
        beginSequence();
    }

private:
    enum class ComplexNodeType : std::uint8_t {
        Mapping, Sequence,
    };

    struct Node {
        ComplexNodeType type = ComplexNodeType::Mapping;
        int indentLevel = 0;
    };

    void pushNode(const ComplexNodeType type)
    {
        if (m_currentNodes.empty()) {
            m_currentNodes.push(Node { type, 0 });
            return;
        }

        const Node& topNode = m_currentNodes.top();

        const int indentLevel = type == ComplexNodeType::Mapping || topNode.type == ComplexNodeType::Sequence
                                ? topNode.indentLevel + 1
                                : topNode.indentLevel;
        m_currentNodes.push(Node { type, indentLevel });
    }

    void popNode(const ComplexNodeType expectedType)
    {
        IF_ASSERT_FAILED(isTopNode(expectedType)) {
            return;
        }

        m_currentNodes.pop();
    }

    template<typename Scalar>
    void writeKeyValue(const std::string_view key, const Scalar& value)
    {
        IF_ASSERT_FAILED(isTopNode(ComplexNodeType::Mapping)) {
            return;
        }

        writeIndent();
        m_textOut << key << ": " << value << '\n';
    }

    bool isTopNode(const ComplexNodeType expectedType)
    {
        return !m_currentNodes.empty() && m_currentNodes.top().type == expectedType;
    }

    void writeIndent()
    {
        if (m_currentNodes.empty()) {
            return;
        }

        const int indentLevel = m_currentNodes.top().indentLevel;
        for (int i = 0; i < indentLevel; ++i) {
            m_textOut << "  ";
        }
    }

    TextStream m_textOut;
    std::stack<Node, std::vector<Node> > m_currentNodes;
};

static std::string_view getEventTypeName(const int type)
{
    switch (type) {
    case engraving::ME_NOTEOFF:
        return "NoteOff"sv;
    case engraving::ME_NOTEON:
        return "NoteOn"sv;
    case engraving::ME_POLYAFTER:
        return "PolyKeyPressure"sv;
    case engraving::ME_CONTROLLER:
        return "ControlChange"sv;
    case engraving::ME_PROGRAM:
        return "ProgramChange"sv;
    case engraving::ME_AFTERTOUCH:
        return "ChannelPressure"sv;
    case engraving::ME_PITCHBEND:
        return "PitchBend"sv;
    case engraving::ME_SYSEX:
        return "SystemExclusive"sv;
    case engraving::ME_ENDSYSEX:
        return "EndofExclusive"sv;
    case engraving::ME_META:
        return "Meta"sv;
    default:
        return "Unknown"sv;
    }
}

static std::string_view getMetaEventTypeName(const int metaType)
{
    switch (metaType) {
    case engraving::META_SEQUENCE_NUMBER:
        return "SequenceNumber"sv;
    case engraving::META_TEXT:
        return "TextEvent"sv;
    case engraving::META_COPYRIGHT:
        return "CopyrightNotice"sv;
    case engraving::META_TRACK_NAME:
        return "SequenceOrTrackName"sv;
    case engraving::META_INSTRUMENT_NAME:
        return "InstrumentName"sv;
    case engraving::META_LYRIC:
        return "Lyric"sv;
    case engraving::META_MARKER:
        return "Marker"sv;
    case engraving::META_CUE_POINT:
        return "CuePoint"sv;
    case engraving::META_EOT:
        return "EndofTrack"sv;
    case engraving::META_TEMPO:
        return "SetTempo"sv;
    case engraving::META_TIME_SIGNATURE:
        return "TimeSignature"sv;
    case engraving::META_KEY_SIGNATURE:
        return "KeySignature"sv;
    case engraving::META_SPECIFIC:
        return "SequencerSpecific"sv;
    default:
        return "Unknown"sv;
    }
}

static void serializeMetaEventData(const uint8_t* data, const int len, YamlStreamWriter& yamlOut)
{
    const auto metaData = QByteArray::fromRawData(reinterpret_cast<const char*>(data), len);
    yamlOut.mapToScalar("data", metaData.toBase64().toStdString());
}

static void serializeEvent(const MidiEvent& event, YamlStreamWriter& yamlOut)
{
    yamlOut.mapToScalar("type", getEventTypeName(event.type()));
    if (event.isChannelEvent()) {
        yamlOut.mapToScalar("channel", event.channel());
    } else if (event.type() == engraving::ME_META) {
        yamlOut.mapToScalar("metaType", getMetaEventTypeName(event.metaType()));
    }

    switch (event.type()) {
    case engraving::ME_NOTEOFF:
    case engraving::ME_NOTEON:
    case engraving::ME_POLYAFTER:
        yamlOut.mapToScalar("pitch", event.pitch());
        yamlOut.mapToScalar("velocity", event.velo());
        break;
    case engraving::ME_CONTROLLER:
        yamlOut.mapToScalar("control", event.controller());
        yamlOut.mapToScalar("value", event.value());
        break;
    case engraving::ME_PROGRAM:
        yamlOut.mapToScalar("program", event.dataA());
        break;
    case engraving::ME_AFTERTOUCH:
        yamlOut.mapToScalar("pressure", event.dataA());
        break;
    case engraving::ME_META:
        serializeMetaEventData(event.edata(), event.len(), yamlOut);
        break;
    default:
        yamlOut.mapToScalar("data1", event.dataA());
        yamlOut.mapToScalar("data2", event.dataB());
        break;
    }
}

static void serializeTrack(const MidiTrack& track, YamlStreamWriter& yamlOut)
{
    for (const auto& [dt, event] : track.events()) {
        yamlOut.sequenceAddMapping();
        yamlOut.mapToScalar("deltaTime", dt);

        yamlOut.mapToMapping("event");
        serializeEvent(event, yamlOut);
        yamlOut.endMapping();

        yamlOut.endMapping();
    }
}
}

muse::Ret SmfYamlSerializer::serialize(const std::filesystem::path& midiPath, muse::io::IODevice* yamlOut)
{
    MidiFile midiFileData;
    {
        QFile midiFile(midiPath);
        if (!midiFile.open(QFile::ReadOnly)) {
            return muse::make_ret(Ret::Code::UnknownError, midiFile.errorString());
        }

        midiFileData.read(&midiFile);
    }

    serialize(midiFileData, yamlOut);

    return muse::make_ok();
}

void SmfYamlSerializer::serialize(const MidiFile& midiFile, io::IODevice* out)
{
    YamlStreamWriter yamlOut(out);
    yamlOut.beginMapping();
    yamlOut.mapToScalar("format", midiFile.format());
    yamlOut.mapToScalar("division", midiFile.division());
    yamlOut.mapToScalar("isDivisionIsInTps", midiFile.isDivisionInTps());

    yamlOut.mapToSequence("tracks");
    for (const auto& track : midiFile.tracks()) {
        yamlOut.sequenceAddSequence();
        serializeTrack(track, yamlOut);
        yamlOut.endSequence();
    }
    yamlOut.endSequence();

    yamlOut.endMapping();
}
}
