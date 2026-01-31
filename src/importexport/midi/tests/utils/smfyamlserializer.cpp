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
#include <optional>
#include <sstream>
#include <stack>
#include <string>
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
        mapToScalar(key, value ? "true"sv : "false"sv);
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
        IF_ASSERT_FAILED(isTopNode(ComplexNodeType::Sequence)) {
            return;
        }

        writeIndent();
        m_textOut << "-\n";
        beginMapping();
    }

    void sequenceAddSequence()
    {
        IF_ASSERT_FAILED(isTopNode(ComplexNodeType::Sequence)) {
            return;
        }

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

        // use zero-indented sequences when used as values in a key-value pair
        const int indentLevel = type == ComplexNodeType::Sequence && topNode.type == ComplexNodeType::Mapping
                                ? topNode.indentLevel
                                : topNode.indentLevel + 1;
        m_currentNodes.push(Node { type, indentLevel });
    }

    void popNode(const ComplexNodeType expectedType)
    {
        IF_ASSERT_FAILED(isTopNode(expectedType)) {
            return;
        }

        m_currentNodes.pop();
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

std::optional<std::string_view> getEventTypeName(const int type)
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
        return std::nullopt;
    }
}

std::optional<std::string_view> getMetaEventTypeName(const int metaType)
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
        return std::nullopt;
    }
}

void serializeMetaEventData(const uint8_t* data, const int len, YamlStreamWriter& yamlOut)
{
    const auto metaData = QByteArray::fromRawData(reinterpret_cast<const char*>(data), len);
    yamlOut.mapToScalar("data", metaData.toBase64().toStdString());
}

std::string makeEscapedString(const uint8_t* data, const int len)
{
    std::string str;
    str += '\"';

    for (int i = 0; i < len; ++i) {
        const auto c = static_cast<char>(data[i]);
        if ((c >= '\x00' && c <= '\x1F')
            || c == '\x7F') {
            std::ostringstream escapedStr;
            escapedStr << "\\x";
            escapedStr << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);
            str += escapedStr.str();
        } else {
            str += c;
        }
    }

    str += '\"';

    return str;
}

void serializeMetaTimeSignature(const std::uint8_t* data, const int len, YamlStreamWriter& yamlOut)
{
    IF_ASSERT_FAILED(len == 4) {
        return;
    }

    yamlOut.mapToScalar("numerator", data[0]);
    yamlOut.mapToScalar("denominatorAsNegPowerOfTwo", data[1]);
    yamlOut.mapToScalar("midiClocksPerClick", data[2]);
    yamlOut.mapToScalar("num32ndNotesPerQuarterNote", data[3]);
}

void serializeMetaKeySignature(const std::uint8_t* data, const int len, YamlStreamWriter& yamlOut)
{
    IF_ASSERT_FAILED(len == 2) {
        return;
    }

    yamlOut.mapToScalar("numSharpsOrFlats", static_cast<std::int8_t>(data[0]));
    yamlOut.mapToScalar("majorOrMinor", data[1]);
}

void serializeMetaTempo(const std::uint8_t* data, const int len, YamlStreamWriter& yamlOut)
{
    IF_ASSERT_FAILED(len == 3) {
        return;
    }

    std::uint32_t usPerQuarterNote = 0;
    for (int i = 0; i < 3; ++i) {
        usPerQuarterNote <<= 8;
        usPerQuarterNote |= data[i];
    }

    yamlOut.mapToScalar("usPerQuarterNote", usPerQuarterNote);
}

void serializeMetaEvent(const int metaType, const std::uint8_t* data, const int len, YamlStreamWriter& yamlOut)
{
    switch (metaType) {
    case engraving::META_TRACK_NAME:
        yamlOut.mapToScalar("name", makeEscapedString(data, len));
        break;
    case engraving::META_TEMPO:
        serializeMetaTempo(data, len, yamlOut);
        break;
    case engraving::META_TIME_SIGNATURE:
        serializeMetaTimeSignature(data, len, yamlOut);
        break;
    case engraving::META_KEY_SIGNATURE:
        serializeMetaKeySignature(data, len, yamlOut);
        break;
    default:
        serializeMetaEventData(data, len, yamlOut);
        break;
    }
}

void serializeEvent(const MidiEvent& event, YamlStreamWriter& yamlOut)
{
    const std::uint8_t eventType = event.type();

    if (const auto maybeEventName = getEventTypeName(eventType)) {
        yamlOut.mapToScalar("type", *maybeEventName);
    } else {
        yamlOut.mapToScalar("type", eventType);
    }

    if (event.isChannelEvent()) {
        yamlOut.mapToScalar("channel", event.channel());
    } else if (eventType == engraving::ME_META) {
        if (const auto maybeMetaName = getMetaEventTypeName(event.metaType())) {
            yamlOut.mapToScalar("metaType", *maybeMetaName);
        } else {
            yamlOut.mapToScalar("metaType", event.metaType());
        }
    }

    switch (eventType) {
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
        serializeMetaEvent(event.metaType(), event.edata(), event.len(), yamlOut);
        break;
    default:
        yamlOut.mapToScalar("data1", event.dataA());
        yamlOut.mapToScalar("data2", event.dataB());
        break;
    }
}

void serializeTrack(const MidiTrack& track, YamlStreamWriter& yamlOut)
{
    for (const auto& [t, event] : track.events()) {
        yamlOut.sequenceAddMapping();
        yamlOut.mapToScalar("time", t);

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

        if (!midiFileData.read(&midiFile)) {
            return muse::make_ret(Ret::Code::UnknownError, "failed to read midi file"s);
        }
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
