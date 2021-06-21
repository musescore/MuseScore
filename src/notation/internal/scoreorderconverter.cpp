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
#include "scoreorderconverter.h"
#include "libmscore/scoreorder.h"
#include "libmscore/score.h"
#include "libmscore/xml.h"
#include <QBuffer>

using namespace mu::notation;
using namespace mu::instruments;

Ms::ScoreOrder ScoreOrderConverter::convertScoreOrder(const mu::instruments::ScoreOrder& order)
{
    Ms::ScoreOrder result;

    result.id = order.id;
    result.name = order.name;

    QMapIterator<QString, InstrumentOverwrite> i(order.instrumentMap);
    while (i.hasNext()) {
        i.next();
        Ms::InstrumentOverwrite io;
        io.id = i.value().id;
        io.name = i.value().name;
        result.instrumentMap.insert(i.key(), io);
    }

    for (auto& osg: order.groups) {
        Ms::ScoreGroup nsg;
        nsg.family = osg.family;
        nsg.section = osg.section;
        nsg.unsorted = osg.unsorted;
        nsg.bracket = osg.bracket;
        nsg.showSystemMarkings = osg.showSystemMarkings;
        nsg.barLineSpan = osg.barLineSpan;
        nsg.thinBracket = osg.thinBracket;
        result.groups << nsg;
    }

    return result;
}

mu::instruments::ScoreOrder ScoreOrderConverter::convertScoreOrder(const Ms::ScoreOrder& order)
{
    ScoreOrder result;

    result.index = -1;
    result.id = order.id;
    result.name = order.name;

    QMapIterator<QString, Ms::InstrumentOverwrite> i(order.instrumentMap);
    while (i.hasNext()) {
        i.next();
        InstrumentOverwrite io;
        io.id = i.value().id;
        io.name = i.value().name;
        result.instrumentMap.insert(i.key(), io);
    }

    for (auto& osg: order.groups) {
        ScoreOrderGroup nsg;
        nsg.family = osg.family;
        nsg.section = osg.section;
        nsg.unsorted = osg.unsorted;
        nsg.bracket = osg.bracket;
        nsg.showSystemMarkings = osg.showSystemMarkings;
        nsg.barLineSpan = osg.barLineSpan;
        nsg.thinBracket = osg.thinBracket;
        result.groups << nsg;
    }

    return result;
}
