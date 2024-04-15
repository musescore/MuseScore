/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_SCOREORDER_H
#define MU_ENGRAVING_SCOREORDER_H

#include <map>

#include "types/translatablestring.h"
#include "../types/types.h"

namespace mu::engraving {
class XmlWriter;
class XmlReader;
class InstrumentTemplate;
class Score;

//---------------------------------------------------------
//   ScoreGroup
//---------------------------------------------------------

struct ScoreGroup
{
    String family;
    String section;
    String unsorted;            // isEmpty()  : equal to <unsorted/>
                                // !isEmpty() : equal to <unsorted group="unsorted"/>
    bool notUnsorted = true;    // not an unsorted group

    bool bracket = false;
    bool barLineSpan = true;
    bool thinBracket = true;
};

//---------------------------------------------------------
//   InstrumentOverwrite
//---------------------------------------------------------

struct InstrumentOverwrite
{
    String id;
    String name;
};

//---------------------------------------------------------
//   ScoreOrder
//---------------------------------------------------------

struct ScoreOrder
{
    String id;
    TranslatableString name;
    std::map<String, InstrumentOverwrite> instrumentMap;
    std::vector<ScoreGroup> groups;
    bool customized = false;

    ScoreOrder() = default;

    ScoreOrder clone() const;
    bool operator==(const ScoreOrder& order) const;
    bool operator!=(const ScoreOrder& order) const;

    bool readBoolAttribute(XmlReader& reader, const char* name, bool defValue);
    void readInstrument(XmlReader& reader);
    void readSoloists(XmlReader& reader, const String section);
    void readSection(XmlReader& reader);
    bool hasGroup(const String& id, const String& group=String()) const;

    bool isValid() const;
    bool isCustom() const;
    TranslatableString getName() const;
    String getFamilyName(const InstrumentTemplate* instrTemplate, bool soloist) const;
    ScoreGroup newUnsortedGroup(const String group, const String section) const;
    ScoreGroup getGroup(const String family, const String instrumentGroup) const;
    int instrumentSortingIndex(const String& instrumentId, bool isSoloist) const;
    bool isScoreOrder(const std::list<int>& indices) const;
    bool isScoreOrder(const Score* score) const;

    void setBracketsAndBarlines(Score* score);

    void read(XmlReader& reader);
    void write(XmlWriter& xml) const;

    void updateInstruments(const Score* score);
};
} // namespace mu::engraving

#endif
