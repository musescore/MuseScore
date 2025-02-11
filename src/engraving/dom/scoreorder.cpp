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
#include "scoreorder.h"

#include <iostream>

#include "rw/xmlreader.h"
#include "rw/xmlwriter.h"

#include "types/translatablestring.h"

#include "dom/bracketItem.h"
#include "dom/instrtemplate.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/staff.h"
#include "dom/undo.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
static const String SOLOISTS_ID(u"<soloists>");
static const String UNSORTED_ID(u"<unsorted>");

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

ScoreOrder ScoreOrder::clone() const
{
    ScoreOrder newOrder;

    newOrder.id = id;
    newOrder.name = name;
    newOrder.customized = customized;
    newOrder.instrumentMap = instrumentMap;

    for (const ScoreGroup& sg : groups) {
        ScoreGroup newGroup;

        newGroup.family = sg.family;
        newGroup.section = sg.section;
        newGroup.unsorted = sg.unsorted;
        newGroup.notUnsorted = sg.notUnsorted;
        newGroup.bracket = sg.bracket;
        newGroup.barLineSpan = sg.barLineSpan;
        newGroup.thinBracket = sg.thinBracket;

        newOrder.groups.push_back(newGroup);
    }

    return newOrder;
}

//---------------------------------------------------------
//   readBoolAttribute
//---------------------------------------------------------

bool ScoreOrder::operator==(const ScoreOrder& order) const
{
    return id == order.id;
}

bool ScoreOrder::operator!=(const ScoreOrder& order) const
{
    return !(*this == order);
}

bool ScoreOrder::readBoolAttribute(XmlReader& reader, const char* attrName, bool defvalue)
{
    if (!reader.hasAttribute(attrName)) {
        return defvalue;
    }
    String attr { reader.attribute(attrName) };
    if (attr.toLower() == "false") {
        return false;
    } else if (attr.toLower() == "true") {
        return true;
    }
    LOGD("invalid value \"%s\" for attribute \"%s\", using default \"%d\"", muPrintable(attr), attrName, defvalue);
    return defvalue;
}

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

void ScoreOrder::readInstrument(XmlReader& reader)
{
    String instrumentId { reader.attribute("id") };
    if (!mu::engraving::searchTemplate(instrumentId)) {
        LOGD("cannot find instrument templates for <%s>", muPrintable(instrumentId));
        reader.skipCurrentElement();
        return;
    }
    while (reader.readNextStartElement()) {
        if (reader.name() == "family") {
            InstrumentOverwrite io;
            io.id = reader.attribute("id");
            io.name = reader.readText();
            instrumentMap.insert({ instrumentId, io });
        } else {
            reader.unknown();
        }
    }
}

//---------------------------------------------------------
//   readSoloists
//---------------------------------------------------------

void ScoreOrder::readSoloists(XmlReader& reader, const String section)
{
    reader.skipCurrentElement();
    if (hasGroup(SOLOISTS_ID)) {
        return;
    }
    ScoreGroup sg;
    sg.family = String(SOLOISTS_ID);
    sg.section = section;
    groups.push_back(sg);
}

//---------------------------------------------------------
//   readSection
//---------------------------------------------------------

void ScoreOrder::readSection(XmlReader& reader)
{
    String sectionId { reader.attribute("id") };
    bool barLineSpan = readBoolAttribute(reader, "barLineSpan", true);
    bool thinBrackets = readBoolAttribute(reader, "thinBrackets", true);
    while (reader.readNextStartElement()) {
        if (reader.name() == "family") {
            ScoreGroup sg;
            sg.family = reader.readText();
            sg.section = sectionId;
            sg.bracket = true;
            sg.barLineSpan = barLineSpan;
            sg.thinBracket = thinBrackets;
            groups.push_back(sg);
        } else if (reader.name() == "unsorted") {
            String group { reader.attribute("group", String()) };

            if (hasGroup(UNSORTED_ID, group)) {
                reader.skipCurrentElement();
                return;
            }

            ScoreGroup sg;
            sg.family = String(UNSORTED_ID);
            sg.section = sectionId;
            sg.unsorted = group;
            sg.notUnsorted = false;
            sg.bracket = true;
            sg.barLineSpan = readBoolAttribute(reader, "barLineSpan", true);
            sg.thinBracket = readBoolAttribute(reader, "thinBrackets", true);
            groups.push_back(sg);
            reader.skipCurrentElement();
        } else {
            reader.unknown();
        }
    }
}

//---------------------------------------------------------
//   hasGroup
//---------------------------------------------------------

bool ScoreOrder::hasGroup(const String& familyId, const String& group) const
{
    for (const ScoreGroup& sg : groups) {
        if ((sg.family == familyId) && (group == sg.unsorted)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   isValid
//---------------------------------------------------------

bool ScoreOrder::isValid() const
{
    return !id.empty();
}

//---------------------------------------------------------
//   isCustom
//---------------------------------------------------------

bool ScoreOrder::isCustom() const
{
    return id == String(u"custom");
}

//---------------------------------------------------------
//   getName
//---------------------------------------------------------

TranslatableString ScoreOrder::getName() const
{
    return customized ? TranslatableString("engraving/scoreorder", "%1 (Customized)").arg(name) : name;
}

//---------------------------------------------------------
//   getFamilyName
//---------------------------------------------------------

String ScoreOrder::getFamilyName(const InstrumentTemplate* instrTemplate, bool soloist) const
{
    if (!instrTemplate) {
        return String(u"<unsorted>");
    }

    if (soloist) {
        return String(u"<soloists>");
    } else if (muse::contains(instrumentMap, instrTemplate->id)) {
        return instrumentMap.at(instrTemplate->id).id;
    } else if (instrTemplate->family) {
        return instrTemplate->family->id;
    }
    return String(u"<unsorted>");
}

//---------------------------------------------------------
//   newUnsortedGroup
//---------------------------------------------------------

ScoreGroup ScoreOrder::newUnsortedGroup(const String group, const String section) const
{
    ScoreGroup sg;
    sg.family = String(UNSORTED_ID);
    sg.section = section;
    sg.unsorted = group;
    sg.notUnsorted = false;
    sg.bracket = false;
    sg.barLineSpan = false;
    sg.thinBracket = false;
    return sg;
}

//---------------------------------------------------------
//   getGroup
//---------------------------------------------------------

ScoreGroup ScoreOrder::getGroup(const String family, const String instrumentGroup) const
{
    static const String UNSORTED = String(u"<unsorted>");

    ScoreGroup unsortedScoreGroup;
    for (const ScoreGroup& sg : groups) {
        if ((sg.family == UNSORTED) && sg.unsorted.isEmpty()) {
            unsortedScoreGroup = sg;
            break;
        }
    }

    if (family.isEmpty()) {
        return unsortedScoreGroup;
    }

    for (const ScoreGroup& sg : groups) {
        if (sg.family == family) {
            return sg;
        }
        if ((sg.family == UNSORTED) && (sg.unsorted == instrumentGroup)) {
            unsortedScoreGroup = sg;
        }
    }
    return unsortedScoreGroup;
}

//---------------------------------------------------------
//   instrumentSortingIndex
//---------------------------------------------------------

int ScoreOrder::instrumentSortingIndex(const String& instrumentId, bool isSoloist) const
{
    static const String SoloistsGroup(u"<soloists>");
    static const String UnsortedGroup(u"<unsorted>");

    enum class Priority : unsigned char {
        Undefined,
        Unsorted,
        UnsortedGroup,
        Family,
        Soloist
    };

    InstrumentIndex ii { searchTemplateIndexForId(instrumentId) };
    if (!ii.instrTemplate) {
        return 0;
    }

    String family = muse::contains(instrumentMap, instrumentId) ? instrumentMap.at(instrumentId).id : ii.instrTemplate->familyId();

    size_t index = groups.size();

    auto calculateIndex = [instrumentId, &ii](size_t index) {
        return index * ii.templateCount + ii.instrTemplate->sequenceOrder;
    };

    Priority priority = Priority::Undefined;

    for (size_t i = 0; i < groups.size(); ++i) {
        const ScoreGroup& sg = groups.at(i);

        if ((sg.family == SoloistsGroup) && isSoloist) {
            return static_cast<int>(calculateIndex(i));
        } else if ((priority < Priority::Family) && (sg.family == family)) {
            index = i;
            priority = Priority::Family;
        } else if ((priority < Priority::UnsortedGroup) && (sg.family == UnsortedGroup)
                   && (sg.unsorted == ii.instrTemplate->groupId)) {
            index = i;
            priority = Priority::UnsortedGroup;
        } else if ((priority < Priority::Unsorted) && (sg.family == UnsortedGroup)
                   && (sg.unsorted.empty())) {
            index = i;
            priority = Priority::Unsorted;
        }
    }

    return static_cast<int>(calculateIndex(index));
}

//---------------------------------------------------------
//   isScoreOrder
//---------------------------------------------------------

bool ScoreOrder::isScoreOrder(const std::list<int>& indices) const
{
    if (isCustom()) {
        return true;
    }

    int prvIndex { -1 };
    for (int curIndex : indices) {
        if (curIndex < prvIndex) {
            return false;
        }
        prvIndex = curIndex;
    }
    return true;
}

bool ScoreOrder::isScoreOrder(const Score* score) const
{
    std::list<int> indices;
    for (const Part* part : score->parts()) {
        indices.push_back(instrumentSortingIndex(part->instrument()->id(), part->soloist()));
    }

    return isScoreOrder(indices);
}

//---------------------------------------------------------
//   setBracketsAndBarlines
//---------------------------------------------------------

void ScoreOrder::setBracketsAndBarlines(Score* score)
{
    if (groups.size() <= 0) {
        return;
    }

    bool prvThnBracket { false };
    bool prvBarLineSpan { false };
    String prvSection;
    int prvInstrument { -1 };
    Staff* prvStaff { nullptr };

    Staff* thkBracketStaff { nullptr };
    Staff* thnBracketStaff { nullptr };
    int thkBracketSpan { 0 };
    int thnBracketSpan { 0 };

    for (Part* part : score->parts()) {
        InstrumentIndex ii = searchTemplateIndexForId(part->instrument()->id());
        if (!ii.instrTemplate) {
            continue;
        }

        String family { getFamilyName(ii.instrTemplate, part->soloist()) };
        const ScoreGroup sg = getGroup(family, instrumentGroups[ii.groupIndex]->id);

        size_t staffIdx { 0 };
        bool blockThinBracket { false };
        size_t braceSpan { 0 };
        for (Staff* staff : part->staves()) {
            // Create copy, because the original is modified while we are iterating over it
            std::vector<BracketItem*> brackets = staff->brackets();

            for (BracketItem* bi : brackets) {
                if (bi->bracketType() == BracketType::BRACE) {
                    braceSpan = std::max(braceSpan, bi->bracketSpan() - 1);
                }
                if (!braceSpan) {
                    score->undo(new RemoveBracket(staff, bi->column(), bi->bracketType(), bi->bracketSpan()));
                }
            }
            if (!braceSpan) {
                staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, 0);
            } else {
                --braceSpan;
            }

            if (prvSection.isEmpty() || (sg.section != prvSection)) {
                if (thkBracketStaff && (thkBracketSpan > 1)) {
                    score->undoAddBracket(thkBracketStaff, 0, BracketType::NORMAL, thkBracketSpan);
                }
                if (!staffIdx) {
                    thkBracketStaff = sg.bracket ? staff : nullptr;
                    thkBracketSpan  = 0;
                }
            }
            if (sg.bracket && !staffIdx) {
                thkBracketSpan += static_cast<int>(part->nstaves());
            }

            if (prvInstrument == -1 || (ii.instrIndex != prvInstrument)) {
                if (thnBracketStaff && (thnBracketSpan > 1)) {
                    score->undoAddBracket(thnBracketStaff, 1, BracketType::SQUARE, thnBracketSpan);
                }
                if (ii.instrIndex != prvInstrument) {
                    thnBracketStaff = (sg.thinBracket && !blockThinBracket) ? staff : nullptr;
                    thnBracketSpan  = 0;
                }
            }

            if (ii.instrTemplate->staffCount > 1) {
                blockThinBracket = true;
                if (staffIdx < ii.instrTemplate->staffCount) {
                    ++staffIdx;
                }
                prvStaff = nullptr;
            } else {
                if (sg.thinBracket && !staffIdx) {
                    thnBracketSpan += static_cast<int>(part->nstaves());
                }
                if (prvStaff) {
                    bool oldBarlineSpan = prvStaff->getProperty(Pid::STAFF_BARLINE_SPAN).toBool();
                    bool newBarlineSpan = prvBarLineSpan && (!prvSection.isEmpty() && (sg.section == prvSection));
                    if (oldBarlineSpan != newBarlineSpan) {
                        prvStaff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, newBarlineSpan);
                    }
                }
                prvStaff = staff;
                ++staffIdx;
            }
            prvSection = sg.section;
            prvBarLineSpan = sg.barLineSpan;
            prvThnBracket = sg.thinBracket;
        }

        prvInstrument = ii.instrIndex;
    }

    if (thkBracketStaff && (thkBracketSpan > 1)) {
        score->undoAddBracket(thkBracketStaff, 0, BracketType::NORMAL, thkBracketSpan);
    }
    if (thnBracketStaff && (thnBracketSpan > 1) && prvThnBracket) {
        score->undoAddBracket(thnBracketStaff, 1, BracketType::SQUARE, thnBracketSpan);
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreOrder::read(XmlReader& reader)
{
    id = reader.attribute("id");
    const String sectionId;
    while (reader.readNextStartElement()) {
        if (reader.name() == "name") {
            name = TranslatableString("engraving/scoreorder", reader.readText());
        } else if (reader.name() == "section") {
            readSection(reader);
        } else if (reader.name() == "instrument") {
            readInstrument(reader);
        } else if (reader.name() == "family") {
            ScoreGroup sg;
            sg.family = reader.readText();
            sg.section = sectionId;
            sg.bracket = false;
            sg.barLineSpan = false;
            sg.thinBracket = false;
            groups.push_back(sg);
        } else if (reader.name() == "soloists") {
            readSoloists(reader, sectionId);
        } else if (reader.name() == "unsorted") {
            String group { reader.attribute("group", String()) };

            if (!hasGroup(UNSORTED_ID, group)) {
                groups.push_back(newUnsortedGroup(group, sectionId));
            }

            reader.skipCurrentElement();
        } else {
            reader.unknown();
        }
    }

    String group;
    if (!hasGroup(UNSORTED_ID, group)) {
        groups.push_back(newUnsortedGroup(group, id));
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ScoreOrder::write(XmlWriter& xml) const
{
    if (!isValid()) {
        return;
    }

    xml.startElement("Order", { { "id", id } });
    xml.tag("name", name.str);

    for (const auto& p : instrumentMap) {
        xml.startElement("instrument", { { "id", p.first } });
        xml.tag("family", { { "id", p.second.id } }, p.second.name);
        xml.endElement();
    }

    String section { u"" };
    for (const ScoreGroup& sg : groups) {
        if (sg.section != section) {
            if (!section.isEmpty()) {
                xml.endElement();
            }
            if (!sg.section.isEmpty()) {
                xml.startElement("section", { { "id", sg.section },
                                     { "brackets", sg.bracket ? "true" : "false" },
                                     { "barLineSpan", sg.barLineSpan ? "true" : "false" },
                                     { "thinBrackets", sg.thinBracket ? "true" : "false" } });
            }
            section = sg.section;
        }
        if (sg.family == SOLOISTS_ID) {
            xml.tag("soloists");
        } else if (sg.notUnsorted) {
            xml.tag("family", sg.family);
        } else if (sg.unsorted.isEmpty()) {
            xml.tag("unsorted");
        } else {
            xml.tag("unsorted", { { "group", sg.unsorted } });
        }
    }
    if (!section.isEmpty()) {
        xml.endElement();
    }
    xml.endElement();
}

//---------------------------------------------------------
//   updateInstruments
//---------------------------------------------------------

void ScoreOrder::updateInstruments(const Score* score)
{
    for (const Part* part : score->parts()) {
        InstrumentIndex ii = searchTemplateIndexForId(part->instrument()->id());
        if (!ii.instrTemplate || !ii.instrTemplate->family) {
            continue;
        }

        InstrumentOverwrite io;
        io.id = ii.instrTemplate->family->id;
        io.name = ii.instrTemplate->family->name;
        instrumentMap.emplace(ii.instrTemplate->id, std::move(io));
    }
}
}
