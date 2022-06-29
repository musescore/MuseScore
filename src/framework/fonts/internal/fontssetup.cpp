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
#include "fontssetup.h"

#include <QFontDatabase>
#include <QCoreApplication>

#include "log.h"

using namespace mu::fonts;

constexpr int INVALID_LOAD_FONTS_CODE = -1;

void FontsSetup::setup()
{
    static const QStringList fonts = {
        ":/fonts/musejazz/MuseJazzText.otf",
        ":/fonts/campania/Campania.otf",
        ":/fonts/edwin/Edwin-Roman.otf",
        ":/fonts/edwin/Edwin-Bold.otf",
        ":/fonts/edwin/Edwin-Italic.otf",
        ":/fonts/edwin/Edwin-BdIta.otf",
        ":/fonts/FreeSans.ttf",
        ":/fonts/FreeSerif.ttf",
        ":/fonts/FreeSerifBold.ttf",
        ":/fonts/FreeSerifItalic.ttf",
        ":/fonts/FreeSerifBoldItalic.ttf",
        ":/fonts/mscoreTab.ttf",
        ":/fonts/mscore-BC.ttf",
        ":/fonts/leland/LelandText.otf",
        ":/fonts/leland/Leland.otf",
        ":/fonts/mscore/MusescoreIcon.ttf",
        ":/fonts/bravura/BravuraText.otf",
        ":/fonts/gootville/GootvilleText.otf",
        ":/fonts/mscore/MScoreText.ttf",
        ":/fonts/petaluma/PetalumaText.otf",
        ":/fonts/petaluma/PetalumaScript.otf",
        ":/fonts/finalemaestro/FinaleMaestroText.otf",
        ":/fonts/finalebroadway/FinaleBroadwayText.otf",
    };

    for (const QString& font: fonts) {
        int loadStatusCode = QFontDatabase::addApplicationFont(font);
        if (loadStatusCode == INVALID_LOAD_FONTS_CODE) {
            LOGE() << "Fatal error: cannot load internal font " << font;
            QCoreApplication::exit(loadStatusCode);
        }
    }
}
