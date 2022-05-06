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
#include "smuflranges.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

//---------------------------------------------------------
//   smuflRanges
//    read smufl ranges.json file
//---------------------------------------------------------

QMap<QString, QStringList>* mu::smuflRanges()
{
    static QMap<QString, QStringList> ranges;
    QStringList allSymbols;

    if (ranges.empty()) {
        QFile fi(":fonts/smufl/ranges.json");
        if (!fi.open(QIODevice::ReadOnly)) {
            LOGD("ScoreFont: open ranges file <%s> failed", qPrintable(fi.fileName()));
        }
        QJsonParseError error;
        QJsonObject o = QJsonDocument::fromJson(fi.readAll(), &error).object();
        if (error.error != QJsonParseError::NoError) {
            LOGD("Json parse error in <%s>(offset: %d): %s", qPrintable(fi.fileName()),
                   error.offset, qPrintable(error.errorString()));
        }

        for (auto s : o.keys()) {
            QJsonObject range = o.value(s).toObject();
            QString desc      = range.value("description").toString();
            QJsonArray glyphs = range.value("glyphs").toArray();
            if (glyphs.size() > 0) {
                QStringList glyphNames;
                for (QJsonValue g : glyphs) {
                    glyphNames.append(g.toString());
                }
                ranges.insert(desc, glyphNames);
                allSymbols << glyphNames;
            }
        }
        ranges.insert(SMUFL_ALL_SYMBOLS, allSymbols);     // TODO: make translatable as well as ranges.json
    }
    return &ranges;
}
