/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef __SCOREORDER_H__
#define __SCOREORDER_H__

#include <map>

#include "libmscore/mscore.h"
#include "instrtemplate.h"

namespace mu::engraving {
//---------------------------------------------------------
//   ScoreGroup
//---------------------------------------------------------

struct ScoreGroup
{
    QString family { QString("") };
    QString section { QString("") };
    QString unsorted { QString() };    // isNull()   : not an unsorted group
                                       // isEmpty()  : equal to <unsorted/>
                                       // !isEmpty() : equal to <unsorted group="unsorted"/>
    bool bracket { false };
    bool barLineSpan { true };
    bool thinBracket { true };
};

//---------------------------------------------------------
//   InstrumentOverwrite
//---------------------------------------------------------

struct InstrumentOverwrite
{
    QString id;
    QString name;
};

//---------------------------------------------------------
//   ScoreOrder
//---------------------------------------------------------

struct ScoreOrder
{
    QString id { QString() };
    QString name { QString() };
    std::map<QString, InstrumentOverwrite> instrumentMap;
    std::vector<ScoreGroup> groups;
    bool customized = false;

    ScoreOrder() = default;

    ScoreOrder clone() const;
    bool operator==(const ScoreOrder& order) const;
    bool operator!=(const ScoreOrder& order) const;

    bool readBoolAttribute(XmlReader& reader, const char* name, bool defValue);
    void readInstrument(XmlReader& reader);
    void readSoloists(XmlReader& reader, const QString section);
    void readSection(XmlReader& reader);
    bool hasGroup(const QString& id, const QString& group=QString()) const;

    bool isValid() const;
    bool isCustom() const;
    QString getName() const;
    QString getFamilyName(const InstrumentTemplate* instrTemplate, bool soloist) const;
    ScoreGroup newUnsortedGroup(const QString group, const QString section) const;
    ScoreGroup getGroup(const QString family, const QString instrumentGroup) const;
    int instrumentSortingIndex(const QString& instrumentId, bool isSoloist) const;
    bool isScoreOrder(const std::list<int>& indices) const;
    bool isScoreOrder(const Score* score) const;

    void setBracketsAndBarlines(Score* score);

    void read(XmlReader& reader);
    void write(XmlWriter& xml) const;

    void updateInstruments(const Score* score);
};
} // namespace mu::engraving

#endif
