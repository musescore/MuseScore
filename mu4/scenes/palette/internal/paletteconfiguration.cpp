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

#include "mscore/globals.h"

#include "settings.h"

using namespace mu::scene::palette;
using namespace mu::framework;

static const Settings::Key PALETTE_SCALE("palette", "application/paletteScale");
static const Settings::Key PALETTE_USESINGLE("palette", "application/useSinglePalette");

void PaletteConfiguration::init()
{
}

double PaletteConfiguration::guiScale() const
{
    double pref = 1.0;
    Val val = settings()->value(PALETTE_SCALE);
    if (!val.isNull()) {
        pref = val.toDouble();
    }

    if (Ms::guiScaling <= 1.0) {                    // low DPI: target is 100% life size
        return pref * Ms::guiScaling;
    } else if (Ms::guiScaling > 1.33) {             // high DPI: target is 75% life size
        return pref * Ms::guiScaling* 0.75;
    } else {                                    // medium high DPI: no target, scaling dependent on resolution
        return pref;                            // (will be 75-100% range)
    }
}

bool PaletteConfiguration::isSinglePalette() const
{
    return settings()->value(PALETTE_USESINGLE).toBool();
}
