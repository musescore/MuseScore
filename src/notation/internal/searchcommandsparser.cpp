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
#include "searchcommandsparser.h"

using namespace mu::notation;

static constexpr QStringView REHEARSAL_MARK_CODE(u"r");
static constexpr QStringView PAGE_CODE(u"p");

SearchCommandsParser::SearchData SearchCommandsParser::parse(const QString& searchCommand)
{
    SearchData data;
    if (searchCommand.isEmpty()) {
        return data;
    }

    data = parseMeasureCommand(searchCommand);
    if (data.isValid()) {
        return data;
    }

    data = parsePageCommand(searchCommand);
    if (data.isValid()) {
        return data;
    }

    data = parseRehearsalMarkCommand(searchCommand);
    return data;
}

SearchCommandsParser::SearchData SearchCommandsParser::parseMeasureCommand(const QString& searchCommand)
{
    QStringList parts = searchCommand.split(u'-', Qt::SkipEmptyParts);

    if (parts.size() == 2) {
        bool ok1, ok2 = false;
        size_t startMeasure = parts[0].toUInt(&ok1);
        size_t endMeasure = parts[1].toUInt(&ok2);

        if (ok1 && ok2 && (startMeasure <= endMeasure)) {
            return SearchData::makeMeasureRange(startMeasure, endMeasure);
        }
        return SearchData{};
    } else {
        bool ok = false;
        size_t measureIndex = searchCommand.toUInt(&ok);

        if (ok) {
            return SearchData::makeMeasure(measureIndex);
        }
    }

    return SearchData{};
}

SearchCommandsParser::SearchData SearchCommandsParser::parsePageCommand(const QString& searchCommand)
{
    QString qsearchCommand = searchCommand.toLower();

    if (!qsearchCommand.startsWith(PAGE_CODE) || qsearchCommand.size() <= PAGE_CODE.size()) {
        return SearchData{};
    }

    if (!qsearchCommand[PAGE_CODE.size()].isNumber()) {
        return SearchData{};
    }

    bool ok = false;
    size_t pageIndex = qsearchCommand.mid(PAGE_CODE.size()).toUInt(&ok);

    if (ok) {
        return SearchData::makePage(pageIndex);
    }

    return SearchData{};
}

SearchCommandsParser::SearchData SearchCommandsParser::parseRehearsalMarkCommand(const QString& searchCommand)
{
    QString qsearchCommand = searchCommand.toLower();

    if (qsearchCommand.startsWith(REHEARSAL_MARK_CODE) && qsearchCommand.size() > REHEARSAL_MARK_CODE.size()) {
        return SearchData::makeRehearsalMark(qsearchCommand.mid(REHEARSAL_MARK_CODE.size()));
    }

    return SearchData::makeRehearsalMark(qsearchCommand);
}
