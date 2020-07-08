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
#include "palettemodule.h"

#include <QQmlEngine>

#include "config.h"
#include "modularity/ioc.h"

#include "internal/mu4paletteadapter.h"

#include "view/paletterootmodel.h"
#include "internal/palette/paletteworkspace.h"

#include "libmscore/score.h"
#include "libmscore/sym.h"

using namespace mu::scene::palette;

static void palette_init_qrc()
{
    Q_INIT_RESOURCE(palette);
}

std::string PaletteModule::moduleName() const
{
    return "palette";
}

void PaletteModule::registerExports()
{
#ifdef BUILD_UI_MU4
    framework::ioc()->registerExport<IPaletteAdapter>(moduleName(), new MU4PaletteAdapter());
#endif
}

void PaletteModule::resolveImports()
{
}

void PaletteModule::registerResources()
{
    palette_init_qrc();
}

void PaletteModule::registerUiTypes()
{
    using namespace Ms;

    qmlRegisterUncreatableType<PaletteWorkspace>("MuseScore.Palette", 1, 0, "PaletteWorkspace", "Cannot create");
    qmlRegisterUncreatableType<AbstractPaletteController>("MuseScore.Palette", 1, 0, "PaletteController", "Cannot ...");
    qmlRegisterUncreatableType<PaletteElementEditor>("MuseScore.Palette", 1, 0, "PaletteElementEditor", "Cannot ...");
    qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 1, 0, "PaletteTreeModel",  "Cannot create");
    qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 1, 0, "FilterPaletteTreeModel", "Cannot");

    qmlRegisterType<PaletteRootModel>("MuseScore.Palette", 1, 0, "PaletteRootModel");
}

void PaletteModule::onInit()
{
    using namespace Ms;

    // create a score for internal use
    gscore = new MasterScore();
    gscore->setPaletteMode(true);
    gscore->setMovements(new Movements());
    gscore->setStyle(MScore::baseStyle());

    gscore->style().set(Sid::MusicalTextFont, QString("Bravura Text"));
    ScoreFont* scoreFont = ScoreFont::fontFactory("Bravura");
    gscore->setScoreFont(scoreFont);
    gscore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gscore->spatium()) / SPATIUM20);
}
