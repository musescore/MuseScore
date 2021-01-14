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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "fontscontroller.h"

#include <QFontDatabase>
#include <QCoreApplication>

#include "log.h"

using namespace mu::fonts;

constexpr int INVALID_LOAD_FONTS_CODE = -1;

void FontsController::init()
{
    loadInternalFonts();
}

void FontsController::loadInternalFonts()
{
    // Do not load application specific fonts
    // for MAC, they are in Resources/fonts
#if defined(Q_OS_MAC) || defined(Q_OS_IOS)
    return;
#endif

    static const QStringList fonts = {
        ":/fonts/musejazz/MuseJazzText.otf",
        ":/fonts/campania/Campania.otf",
        ":/fonts/FreeSans.ttf",
        ":/fonts/firasans/FiraSansRegular.ttf",
        ":/fonts/firasans/FiraSansSemiBold.ttf",
        ":/fonts/FreeSerif.ttf",
        ":/fonts/FreeSerifBold.ttf",
        ":/fonts/FreeSerifItalic.ttf",
        ":/fonts/FreeSerifBoldItalic.ttf",
        ":/fonts/mscoreTab.ttf",
        ":/fonts/mscore-BC.ttf",
        ":/fonts/bravura/BravuraText.otf",
        ":/fonts/gootville/GootvilleText.otf",
        ":/fonts/mscore/MScoreText.ttf",
        ":/fonts/mscore/MusescoreIcon.ttf",
        ":/fonts/leland/Leland.otf",
        ":/fonts/petaluma/PetalumaText.otf",
        ":/fonts/petaluma/PetalumaScript.otf",
    };

    for (const QString& font: fonts) {
        int loadStatusCode = QFontDatabase::addApplicationFont(font);
        if (loadStatusCode == INVALID_LOAD_FONTS_CODE) {
            LOGE() << "Fatal error: cannot load internal font " << font;
            QCoreApplication::exit(loadStatusCode);
        }
    }
}
