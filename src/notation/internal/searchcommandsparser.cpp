//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "searchcommandsparser.h"

#include "libmscore/rehearsalmark.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"

#include "log.h"
#include "translation.h"
#include "notationerrors.h"

using namespace mu::notation;

static const std::string REHEARSAL_MARK_CODE("r");
static const std::string PAGE_CODE("p");

SearchCommands SearchCommandsParser::availableCommands()
{
    SearchCommands commands;
    commands << SearchCommand(ElementType::REHEARSAL_MARK, REHEARSAL_MARK_CODE, trc("notation", "Rehearsal marks"))
             << SearchCommand(ElementType::PAGE, PAGE_CODE, trc("notation", "Pages"));

    return commands;
}

SearchCommandsParser::SearchData SearchCommandsParser::parse(const std::string& searchCommand)
{
    SearchData data;
    if (searchCommand.empty()) {
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

SearchCommandsParser::SearchData SearchCommandsParser::parseMeasureCommand(const std::string& searchCommand)
{
    SearchData data;

    QString qsearchCommand = QString::fromStdString(searchCommand).toLower();
    bool ok = false;
    int measureIndex = qsearchCommand.toInt(&ok);

    if (ok) {
        data.elementType = ElementType::MEASURE;
        data.value = QVariant::fromValue(measureIndex);
    }

    return data;
}

SearchCommandsParser::SearchData SearchCommandsParser::parsePageCommand(const std::string& searchCommand)
{
    SearchData data;

    QString qsearchCommand = QString::fromStdString(searchCommand).toLower();

    int pageCodeLength = static_cast<int>(PAGE_CODE.length());
    if (!qsearchCommand.startsWith(PAGE_CODE.c_str()) || qsearchCommand.size() <= pageCodeLength) {
        return data;
    }

    if (!qsearchCommand[pageCodeLength].isNumber()) {
        return data;
    }

    bool ok = false;
    int pageIndex = qsearchCommand.mid(static_cast<int>(PAGE_CODE.size())).toInt(&ok);

    if (ok) {
        data.elementType = ElementType::PAGE;
        data.value = pageIndex;
    }

    return data;
}

SearchCommandsParser::SearchData SearchCommandsParser::parseRehearsalMarkCommand(const std::string& searchCommand)
{
    SearchData data;

    QString qsearchCommand = QString::fromStdString(searchCommand).toLower();

    int rehearsalMarkCodeLength = static_cast<int>(REHEARSAL_MARK_CODE.length());
    if (qsearchCommand.startsWith(REHEARSAL_MARK_CODE.c_str()) && qsearchCommand.size() > rehearsalMarkCodeLength) {
        data.elementType = ElementType::REHEARSAL_MARK;
        data.value = qsearchCommand.mid(rehearsalMarkCodeLength);
        return data;
    }

    data.elementType = ElementType::REHEARSAL_MARK;
    data.value = qsearchCommand;

    return data;
}
