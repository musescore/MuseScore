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
#include "paletteconfiguration.h"

#include "log.h"
#include "settings.h"

using namespace mu::palette;
using namespace mu::framework;

static const Settings::Key PALETTE_SCALE("palette", "application/paletteScale");
static const Settings::Key PALETTE_USE_SINGLE("palette", "application/useSinglePalette");
static const Settings::Key PALETTE_USE_USER_FG_COLOR("palette", "ui/canvas/foreground/useColorInPalettes");

void PaletteConfiguration::init()
{
    settings()->addItem(PALETTE_USE_USER_FG_COLOR, Val(true));
}

double PaletteConfiguration::guiScale() const
{
    double pref = 1.0;
    Val val = settings()->value(PALETTE_SCALE);
    if (!val.isNull()) {
        pref = val.toDouble();
    }

    float guiScaling = uiConfiguration()->guiScaling();

    if (guiScaling <= 1.0) {                    // low DPI: target is 100% life size
        return pref * guiScaling;
    } else if (guiScaling > 1.33) {             // high DPI: target is 75% life size
        return pref * guiScaling * 0.75;
    } else {                                    // medium high DPI: no target, scaling dependent on resolution
        return pref;                            // (will be 75-100% range)
    }
}

bool PaletteConfiguration::isSinglePalette() const
{
    return settings()->value(PALETTE_USE_SINGLE).toBool();
}

QColor PaletteConfiguration::foregroundColor() const
{
    //! NOTE Notation configuration may not exist when building in MU3 mode.
    //! Because there is no `mu::notation` module in this mode.
    //! For this case, let's add a workaround
    if (!notationConfiguration()) {
        static const Settings::Key FOREGROUND_COLOR("notation", "ui/canvas/foreground/color");
        static const Settings::Key FOREGROUND_USE_USER_COLOR("notation", "ui/canvas/foreground/useColor");

        if (settings()->value(PALETTE_USE_USER_FG_COLOR).toBool()) {
            if (settings()->value(FOREGROUND_USE_USER_COLOR).toBool()) {
                return settings()->value(FOREGROUND_COLOR).toQColor();
            }
        }
        return QColor("#f9f9f9"); // default
    }

    if (settings()->value(PALETTE_USE_USER_FG_COLOR).toBool()) {
        return notationConfiguration()->foregroundColor();
    }
    return notationConfiguration()->defaultForegroundColor();
}
