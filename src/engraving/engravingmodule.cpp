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
#include "engravingmodule.h"

#include "modularity/ioc.h"
#include "global/allocator.h"

#include "draw/ifontprovider.h"
#include "infrastructure/smufl.h"
#include "infrastructure/symbolfonts.h"

#ifndef ENGRAVING_NO_INTERNAL
#include "internal/engravingconfiguration.h"
#endif

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "engraving/accessibility/accessibleitem.h"
#endif

#include "engraving/style/defaultstyle.h"

#include "engraving/libmscore/mscore.h"
#include "engraving/libmscore/masterscore.h"

#include "compat/scoreaccess.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::modularity;
using namespace mu::draw;

#ifndef ENGRAVING_NO_INTERNAL
static std::shared_ptr<EngravingConfiguration> s_configuration = std::make_shared<EngravingConfiguration>();
#endif

static void engraving_init_qrc()
{
#ifndef NO_QT_SUPPORT
    Q_INIT_RESOURCE(engraving);
#endif
}

std::string EngravingModule::moduleName() const
{
    return "engraving";
}

void EngravingModule::registerExports()
{
#ifndef ENGRAVING_NO_INTERNAL
    ioc()->registerExport<IEngravingConfiguration>(moduleName(), s_configuration);
#endif
}

void EngravingModule::resolveImports()
{
}

void EngravingModule::registerResources()
{
    engraving_init_qrc();
}

void EngravingModule::registerUiTypes()
{
    MScore::registerUiTypes();
}

void EngravingModule::onInit(const framework::IApplication::RunMode&)
{
    // Init fonts
    {
        // Symbols
        Smufl::init();

        SymbolFonts::addFont(u"Leland",     u"Leland",      ":/fonts/leland/Leland.otf");
        SymbolFonts::addFont(u"Bravura",    u"Bravura",     ":/fonts/bravura/Bravura.otf");
        SymbolFonts::addFont(u"Emmentaler", u"MScore",      ":/fonts/mscore/mscore.ttf");
        SymbolFonts::addFont(u"Gonville",   u"Gootville",   ":/fonts/gootville/Gootville.otf");
        SymbolFonts::addFont(u"MuseJazz",   u"MuseJazz",    ":/fonts/musejazz/MuseJazz.otf");
        SymbolFonts::addFont(u"Petaluma",   u"Petaluma",    ":/fonts/petaluma/Petaluma.otf");
        SymbolFonts::addFont(u"Finale Maestro", u"Finale Maestro", ":/fonts/finalemaestro/FinaleMaestro.otf");
        SymbolFonts::addFont(u"Finale Broadway", u"Finale Broadway", ":/fonts/finalebroadway/FinaleBroadway.otf");

        SymbolFonts::setFallbackFont(u"Bravura");

        // Text
        const std::vector<io::path_t> textFonts = {
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
            ":/fonts/bravura/BravuraText.otf",
            ":/fonts/gootville/GootvilleText.otf",
            ":/fonts/mscore/MScoreText.ttf",
            ":/fonts/petaluma/PetalumaText.otf",
            ":/fonts/petaluma/PetalumaScript.otf",
            ":/fonts/finalemaestro/FinaleMaestroText.otf",
            ":/fonts/finalebroadway/FinaleBroadwayText.otf",
        };

        std::shared_ptr<IFontProvider> fontProvider = ioc()->resolve<IFontProvider>("fonts");
        for (const io::path_t& font : textFonts) {
            int loadStatusCode = fontProvider->addTextFont(font);
            if (loadStatusCode == -1) {
                LOGE() << "Fatal error: cannot load internal font " << font;
            }
        }

        fontProvider->insertSubstitution(u"Leland Text",    u"Bravura Text");
        fontProvider->insertSubstitution(u"Bravura Text",   u"Leland Text");
        fontProvider->insertSubstitution(u"MScore Text",    u"Leland Text");
        fontProvider->insertSubstitution(u"Gootville Text", u"Leland Text");
        fontProvider->insertSubstitution(u"MuseJazz Text",  u"Leland Text");
        fontProvider->insertSubstitution(u"Petaluma Text",  u"MuseJazz Text");
        fontProvider->insertSubstitution(u"Finale Maestro Text", u"Leland Text");
        fontProvider->insertSubstitution(u"Finale Broadway Text", u"MuseJazz Text");
        fontProvider->insertSubstitution(u"ScoreFont",      u"Leland Text");// alias for current Musical Text Font
    }

#ifndef ENGRAVING_NO_INTERNAL
    s_configuration->init();

    DefaultStyle::instance()->init(s_configuration->defaultStyleFilePath(),
                                   s_configuration->partStyleFilePath());
#endif

    MScore::init();     // initialize libmscore

    MScore::setNudgeStep(0.1); // cursor key (default 0.1)
    MScore::setNudgeStep10(1.0); // Ctrl + cursor key (default 1.0)
    MScore::setNudgeStep50(0.01); // Alt  + cursor key (default 0.01)

    // Palette
    {
#ifndef ENGRAVING_NO_ACCESSIBILITY
        AccessibleItem::enabled = false;
#endif
        gpaletteScore = compat::ScoreAccess::createMasterScore();
#ifndef ENGRAVING_NO_ACCESSIBILITY
        AccessibleItem::enabled = true;
#endif

        if (EngravingObject::elementsProvider()) {
            EngravingObject::elementsProvider()->unreg(gpaletteScore);
        }

        gpaletteScore->setStyle(DefaultStyle::baseStyle());

        gpaletteScore->style().set(Sid::MusicalTextFont, String(u"Leland Text"));
        SymbolFont* scoreFont = SymbolFonts::fontByName(u"Leland");
        gpaletteScore->setSymbolFont(scoreFont);
        gpaletteScore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gpaletteScore->spatium()) / SPATIUM20);
    }

    //! NOTE And some initialization in the `Notation::init()`
}

void EngravingModule::onDestroy()
{
    delete gpaletteScore;
    gpaletteScore = nullptr;
}
