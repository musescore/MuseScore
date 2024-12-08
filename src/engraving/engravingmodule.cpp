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
#include "engravingmodule.h"

#include "modularity/ioc.h"
#include "global/allocator.h"

#include "draw/ifontprovider.h"
#include "draw/internal/ifontsdatabase.h"

#include "infrastructure/smufl.h"
#include "infrastructure/localfileinfoprovider.h"

#ifndef ENGRAVING_NO_INTERNAL
#include "internal/engravingconfiguration.h"
#include "internal/engravingfontsprovider.h"
#endif

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "engraving/accessibility/accessibleitem.h"
#endif

#include "engraving/style/defaultstyle.h"

#include "engraving/dom/stafftype.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/fret.h"

#include "rendering/score/scorerenderer.h"
#include "rendering/single/singlerenderer.h"

#include "compat/scoreaccess.h"

#ifndef ENGRAVING_NO_API
#include "global/api/iapiregister.h"
#include "api/v1/qmlpluginapi.h"
#include "api/v1/engravingapiv1.h"
#endif

#ifdef MUE_BUILD_ENGRAVING_DEVTOOLS
#include "ui/iinteractiveuriregister.h"
#include "devtools/engravingelementsprovider.h"
#include "devtools/engravingelementsmodel.h"
#include "devtools/corruptscoredevtoolsmodel.h"
#include "devtools/drawdata/diagnosticdrawprovider.h"
#endif

#include "muse_framework_config.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::modularity;
using namespace muse::draw;

static void engraving_init_qrc()
{
#ifndef NO_QT_SUPPORT
    Q_INIT_RESOURCE(engraving);

    Q_INIT_RESOURCE(fonts_Leland);
    Q_INIT_RESOURCE(fonts_Bravura);
    Q_INIT_RESOURCE(fonts_Campania);
    Q_INIT_RESOURCE(fonts_Edwin);
    Q_INIT_RESOURCE(fonts_FreeSans);
    Q_INIT_RESOURCE(fonts_FreeSerif);
    Q_INIT_RESOURCE(fonts_Gootville);
    Q_INIT_RESOURCE(fonts_MScore);
    Q_INIT_RESOURCE(fonts_MuseJazz);
    Q_INIT_RESOURCE(fonts_Smufl);
    Q_INIT_RESOURCE(fonts_Petaluma);
    Q_INIT_RESOURCE(fonts_FinaleMaestro);
    Q_INIT_RESOURCE(fonts_FinaleBroadway);
    Q_INIT_RESOURCE(fonts_Tabulature);
#endif
}

std::string EngravingModule::moduleName() const
{
    return "engraving";
}

void EngravingModule::registerExports()
{
#ifndef ENGRAVING_NO_INTERNAL

    m_configuration = std::make_shared<EngravingConfiguration>(iocContext());
    m_engravingfonts = std::make_shared<EngravingFontsProvider>(iocContext());

    ioc()->registerExport<IEngravingConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IEngravingFontsProvider>(moduleName(), m_engravingfonts);
#endif

    // internal
    ioc()->registerExport<rendering::IScoreRenderer>(moduleName(), new rendering::score::ScoreRenderer());
    ioc()->registerExport<rendering::ISingleRenderer>(moduleName(), new rendering::single::SingleRenderer());

#ifdef MUE_BUILD_ENGRAVING_DEVTOOLS
    ioc()->registerExport<IEngravingElementsProvider>(moduleName(), new EngravingElementsProvider());
    ioc()->registerExport<IDiagnosticDrawProvider>(moduleName(), new DiagnosticDrawProvider(iocContext()));
#endif
}

void EngravingModule::resolveImports()
{
#ifdef MUE_BUILD_ENGRAVING_DEVTOOLS
    auto ir = ioc()->resolve<muse::ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://diagnostics/engraving/elements"), "MuseScore/Engraving/EngravingElementsDialog.qml");
    }
#endif
}

void EngravingModule::registerApi()
{
#ifndef ENGRAVING_NO_API
    apiv1::PluginAPI::registerQmlTypes();

    auto api = ioc()->resolve<muse::api::IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator(moduleName(), "api.engraving.v1", new muse::api::ApiCreator<apiv1::EngravingApiV1>());
    }
#endif
}

void EngravingModule::registerResources()
{
    engraving_init_qrc();
}

void EngravingModule::registerUiTypes()
{
    MScore::registerUiTypes();

#ifdef MUE_BUILD_ENGRAVING_DEVTOOLS
    qmlRegisterType<EngravingElementsModel>("MuseScore.Engraving", 1, 0, "EngravingElementsModel");
    qmlRegisterType<CorruptScoreDevToolsModel>("MuseScore.Engraving", 1, 0, "CorruptScoreDevToolsModel");
#endif
}

void EngravingModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

#ifndef ENGRAVING_NO_INTERNAL
    // Init fonts
    {
        using namespace muse::draw;

        std::shared_ptr<IFontsDatabase> fdb = ioc()->resolve<IFontsDatabase>(moduleName());

        // Text
        fdb->addFont(FontDataKey(u"Edwin", false, false), ":/fonts/edwin/Edwin-Roman.otf");
        fdb->addFont(FontDataKey(u"Edwin", false, true), ":/fonts/edwin/Edwin-Italic.otf");
        fdb->addFont(FontDataKey(u"Edwin", true, false), ":/fonts/edwin/Edwin-Bold.otf");
        fdb->addFont(FontDataKey(u"Edwin", true, true), ":/fonts/edwin/Edwin-BdIta.otf");

        // MusicSymbol[Text]
        auto addMusicFont = [this, fdb](const std::string& name, const FontDataKey& fontDataKey, const muse::io::path_t& filePath){
            fdb->addFont(FontDataKey(fontDataKey), filePath);
            m_engravingfonts->addInternalFont(name, fontDataKey.family().id().toStdString(), filePath);
        };

        addMusicFont("Bravura", FontDataKey(u"Bravura"), ":/fonts/bravura/Bravura.otf");
        fdb->addFont(FontDataKey(u"Bravura Text"), ":/fonts/bravura/BravuraText.otf");
        addMusicFont("Leland", FontDataKey(u"Leland"), ":/fonts/leland/Leland.otf");
        fdb->addFont(FontDataKey(u"Leland Text"), ":/fonts/leland/LelandText.otf");
        addMusicFont("Emmentaler", FontDataKey(u"MScore"), ":/fonts/mscore/mscore.ttf");
        fdb->addFont(FontDataKey(u"MScore Text"), ":/fonts/mscore/MScoreText.ttf");
        addMusicFont("Gonville", FontDataKey(u"Gootville"), ":/fonts/gootville/Gootville.otf");
        fdb->addFont(FontDataKey(u"Gootville Text"), ":/fonts/gootville/GootvilleText.otf");
        addMusicFont("MuseJazz", FontDataKey(u"MuseJazz"), ":/fonts/musejazz/MuseJazz.otf");
        fdb->addFont(FontDataKey(u"MuseJazz Text"), ":/fonts/musejazz/MuseJazzText.otf");
        addMusicFont("Petaluma", FontDataKey(u"Petaluma"),    ":/fonts/petaluma/Petaluma.otf");
        fdb->addFont(FontDataKey(u"Petaluma Text"), ":/fonts/petaluma/PetalumaText.otf");
        addMusicFont("Finale Maestro", FontDataKey(u"Finale Maestro"), ":/fonts/finalemaestro/FinaleMaestro.otf");
        fdb->addFont(FontDataKey(u"Finale Maestro Text"), ":/fonts/finalemaestro/FinaleMaestroText.otf");
        addMusicFont("Finale Broadway", FontDataKey(u"Finale Broadway"), ":/fonts/finalebroadway/FinaleBroadway.otf");
        fdb->addFont(FontDataKey(u"Finale Broadway Text"), ":/fonts/finalebroadway/FinaleBroadwayText.otf");

        // Tabulature
        fdb->addFont(FontDataKey(u"FreeSerif"), ":/fonts/FreeSerif.ttf");
        fdb->addFont(FontDataKey(u"FreeSerif", true, false), ":/fonts/FreeSerifBold.ttf");
        fdb->addFont(FontDataKey(u"FreeSerif", false, true), ":/fonts/FreeSerifItalic.ttf");
        fdb->addFont(FontDataKey(u"FreeSerif", true, true), ":/fonts/FreeSerifBoldItalic.ttf");
        fdb->addFont(FontDataKey(u"FreeSans"), ":/fonts/FreeSans.ttf");
        fdb->addFont(FontDataKey(u"MScoreTabulature"), ":/fonts/mscoreTab.ttf");

        // Figured Bass
        fdb->addFont(FontDataKey(u"MscoreBC"), ":/fonts/mscore-BC.ttf");

        // Roman Numeral Analysis
        fdb->addFont(FontDataKey(u"Campania"), ":/fonts/campania/Campania.otf");

        // Defaults
        fdb->setDefaultFont(Font::Type::Unknown, FontDataKey(u"Edwin"));
        fdb->setDefaultFont(Font::Type::Text, FontDataKey(u"Edwin"));
        fdb->setDefaultFont(Font::Type::Tablature, FontDataKey(u"FreeSerif"));
        fdb->setDefaultFont(Font::Type::MusicSymbolText, FontDataKey(u"Bravura Text"));
        fdb->setDefaultFont(Font::Type::MusicSymbol, FontDataKey(u"Bravura"));
        m_engravingfonts->setFallbackFont("Bravura");

        //! NOTE Used for Qt font provider
        fdb->insertSubstitution(u"Leland Text",    u"Bravura Text");
        fdb->insertSubstitution(u"Bravura Text",   u"Leland Text");
        fdb->insertSubstitution(u"MScore Text",    u"Leland Text");
        fdb->insertSubstitution(u"Gootville Text", u"Leland Text");
        fdb->insertSubstitution(u"MuseJazz Text",  u"Leland Text");
        fdb->insertSubstitution(u"Petaluma Text",  u"MuseJazz Text");
        fdb->insertSubstitution(u"Finale Maestro Text", u"Leland Text");
        fdb->insertSubstitution(u"Finale Broadway Text", u"MuseJazz Text");
        fdb->insertSubstitution(u"ScoreFont",      u"Leland Text");// alias for current Musical Text Font

        // Symbols
        Smufl::init();

        //! NOTE It may be necessary to draw something with these fonts without requesting the fonts themselves
        //! (for example, simply specifying the family name for painter).
        //! But if they are not loaded, then they are not added to the font database and,
        //! accordingly, they are drawn incorrectly
        m_engravingfonts->loadAllFonts();
    }

    m_configuration->init();

    DefaultStyle::instance()->init(m_configuration->defaultStyleFilePath(),
                                   m_configuration->partStyleFilePath(),
                                   m_configuration->defaultPageSize());

    StaffType::initStaffTypes(m_configuration->defaultColor());
#endif // ENGRAVING_NO_INTERNAL

    // initialize dom

    MScore::defaultPlayDuration = 300;            // ms
    MScore::warnPitchRange      = true;
    MScore::warnGuitarBends     = true;
    MScore::pedalEventsMinTicks = 1;

    Drumset::initDrumset();
    FiguredBass::readConfigFile(String());

    MScore::setNudgeStep(0.1);     // cursor key (default 0.1)
    MScore::setNudgeStep10(1.0);     // Ctrl + cursor key (default 1.0)
    MScore::setNudgeStep50(0.01);     // Alt  + cursor key (default 0.01)

    // Palette
    {
#ifndef ENGRAVING_NO_ACCESSIBILITY
        AccessibleItem::enabled = false;
#endif
        gpaletteScore = compat::ScoreAccess::createMasterScore(iocContext());
        gpaletteScore->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(""));

#ifndef ENGRAVING_NO_ACCESSIBILITY
        AccessibleItem::enabled = true;
#endif

        if (gpaletteScore->elementsProvider()) {
            gpaletteScore->elementsProvider()->unreg(gpaletteScore);
        }

#ifndef ENGRAVING_NO_INTERNAL
        gpaletteScore->setStyle(DefaultStyle::baseStyle());
        gpaletteScore->style().set(Sid::musicalTextFont, String(u"Leland Text"));
        IEngravingFontPtr scoreFont = m_engravingfonts->fontByName("Leland");
        gpaletteScore->setEngravingFont(scoreFont);
        gpaletteScore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gpaletteScore->style().spatium()) / SPATIUM20);
#endif
    }

    //! NOTE And some initialization in the `Notation::init()`
}

void EngravingModule::onDestroy()
{
    delete gpaletteScore;
    gpaletteScore = nullptr;
}
