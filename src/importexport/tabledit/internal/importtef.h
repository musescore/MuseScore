/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
#pragma once

#include <array>
#include <string>
#include <vector>

#include "engraving/dom/masterscore.h"
#include "engraving/engravingerrors.h"
#include "io/iodevice.h"

#include "note.h"
#include "voiceallocator.h"

namespace mu::iex::tabledit {
class MeasureHandler;

// header size
static const size_t HEADER_SIZE = 256;

// offsets into the file header
static const uint8_t OFFSET_SECURITYCODE = 0x10;
static const uint8_t OFFSET_TBED = 0x38;
static const uint8_t OFFSET_CONTENTS = 0x3C;
static const uint8_t OFFSET_TITLE = 0x40;
static const uint8_t OFFSET_SUBTITLE = 0x44;
static const uint8_t OFFSET_COMMENT = 0x48;
static const uint8_t OFFSET_NOTES = 0x4C;
static const uint8_t OFFSET_TEXTS = 0x54;
static const uint8_t OFFSET_MEASURES = 0x5C;
static const uint8_t OFFSET_INSTRUMENTS = 0x60;
static const uint8_t OFFSET_READINGLIST = 0x80;
static const uint8_t OFFSET_INTERNETLINK = 0x84;
static const uint8_t OFFSET_COPYRIGHT = 0x8C;
static const uint8_t OFFSET_OLDNUM = 0xCA;
static const uint8_t OFFSET_WFORMAT = 0xCC;

struct TefMeasure {
    int flag { 0 };
    bool isPickup { false };
    int key { 0 };
    int size { 0 };
    int numerator { 0 };
    int denominator { 0 };
};

struct TefReadingListItem {
    int firstMeasure { 0 };
    int lastMeasure { 0 };
    std::string label;
};

class TablEdit
{
    muse::String _errorMessage;
    muse::io::IODevice* _file = nullptr;
    mu::engraving::MasterScore* score = nullptr;

    int8_t readInt8();
    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt32();
    std::string readUtf8Text();
    std::string readUtf8TextIndirect(uint32_t positionOfPosition);

    struct TefHeader {
        int version { 0 };
        int subVersion { 0 };
        int tempo { 0 };
        int chorus { 0 };
        int reverb { 0 };
        int syncope { 0 };
        unsigned int securityCode { 0 }; // open file only if 0
        unsigned int securityFlags { 0 };
        int tbed { 0 }; // should be 0x64656274 ("tbed"), but is sometimes reversed (?)
        int wOldNum { 0 }; // must be 4
        int wFormat { 0 }; // hibyte > 9: v3.00 file, hibyte > 10: may no longer match v3.00 spec
        std::string title;
        std::string subTitle;
        std::string comment;
        std::string notes;
        std::string internetLink;
        std::string copyright;
    };

    struct TefInstrument {
        int stringNumber { 0 };
        int firstString { 0 };
        int available16U { 0 };
        int verticalSpacing { 0 };
        int midiVoice { 0 };
        int midiBank { 0 };
        int nBanjo5 { 0 };
        int uSpec { 0 };
        int nCapo { 0 };
        int fMiddleC { 0 };
        int fClef { 0 };
        int output { 0 };
        int options { 0 };
        std::array<int, 12> tuning = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        std::string name;
    };

    struct TefTextMarker {
        int position { 0 };
        int string { 0 };
        int index { 0 };
    };

    void allocateVoices(std::vector<VoiceAllocator>& allocator);
    void createCapos();
    void createContents(const MeasureHandler& measureHandler);
    void createEffects();
    void createLinkedTabs();
    void createMeasures(const MeasureHandler& measureHandler);
    void createNotesFrame();
    void createParts();
    void createProperties();
    void createRepeats();
    void createScore();
    void createTempo();
    void createTexts();
    void createTitleFrame();
    void initializeVoiceAllocators(std::vector<VoiceAllocator>& allocators);
    engraving::part_idx_t partIdx(size_t stringIdx, bool& ok) const;
    int stringNumberPreviousParts(engraving::part_idx_t partIdx) const;
    uint32_t readSignature();
    void readTefContents();
    mu::engraving::Err readTefHeader();
    void readTefInstruments();
    void readTefMeasures();
    void readTefReadingList();
    void readTefTexts();
    uint16_t readWFormat();

    TefHeader tefHeader;
    std::vector<TefTextMarker> tefTextMarkers;
    std::vector<TefNote> tefContents; // notes (and rests) only
    std::vector<TefInstrument> tefInstruments;
    std::vector<TefMeasure> tefMeasures;
    std::vector<TefReadingListItem> tefReadingList;
    std::vector<std::string> tefTexts;

    std::map<const TefNote* const, mu::engraving::Note*> effectMap;

public:
    TablEdit(mu::engraving::MasterScore* s)
        : score(s) {}
    mu::engraving::Err import(const muse::io::path_t& path);
    muse::String errorMessage() { return _errorMessage; }
};
} // namespace mu::iex::tabledit
