#include "appfactory.h"

#include "internal/guiapp.h"
#include "internal/consoleapp.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_ACCESSIBILITY
#include "framework/accessibility/accessibilitymodule.h"
#else
#include "framework/stubs/accessibility/accessibilitystubmodule.h"
#endif

#include "framework/actions/actionsmodule.h"

#ifdef MUSE_MODULE_AUDIO
#include "framework/audio/audiomodule.h"
#else
#include "framework/stubs/audio/audiostubmodule.h"
#endif

#ifdef MUSE_MODULE_AUDIOPLUGINS
#include "framework/audioplugins/audiopluginsmodule.h"
#endif

#ifdef MUSE_MODULE_CLOUD
#include "framework/cloud/cloudmodule.h"
#else
#include "framework/stubs/cloud/cloudstubmodule.h"
#endif

#include "framework/draw/drawmodule.h"

#ifdef MUSE_MODULE_LANGUAGES
#include "framework/languages/languagesmodule.h"
#else
#include "framework/stubs/languages/languagesstubmodule.h"
#endif

#ifdef MUSE_MODULE_LEARN
#include "framework/learn/learnmodule.h"
#else
#include "framework/stubs/learn/learnmodule.h"
#endif

#ifdef MUSE_MODULE_MIDI
#include "framework/midi/midimodule.h"
#else
#include "framework/stubs/midi/midistubmodule.h"
#endif

#ifdef MUSE_MODULE_MPE
#include "framework/mpe/mpemodule.h"
#else
#include "framework/stubs/mpe/mpestubmodule.h"
#endif

#ifdef MUSE_MODULE_MULTIINSTANCES
#include "framework/multiinstances/multiinstancesmodule.h"
#else
#include "framework/stubs/multiinstances/multiinstancesstubmodule.h"
#endif

#ifdef MUSE_MODULE_MUSESAMPLER
#include "framework/musesampler/musesamplermodule.h"
#endif

#ifdef MUSE_MODULE_NETWORK
#include "framework/network/networkmodule.h"
#else
#include "framework/stubs/network/networkstubmodule.h"
#endif

#ifdef MUSE_MODULE_SHORTCUTS
#include "framework/shortcuts/shortcutsmodule.h"
#else
#include "framework/stubs/shortcuts/shortcutsstubmodule.h"
#endif

#ifdef MUSE_MODULE_TOURS
#include "framework/tours/toursmodule.h"
#else
#include "framework/stubs/tours/toursstubmodule.h"
#endif

#ifdef MUSE_MODULE_UI
#include "framework/dockwindow/dockmodule.h"
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#endif

#ifdef MUSE_MODULE_UPDATE
#include "update/updatemodule.h"
#else
#include "framework/stubs/update/updatestubmodule.h"
#endif

#ifdef MUSE_MODULE_VST
#include "framework/vst/vstmodule.h"
#else
#include "framework/stubs/vst/vststubmodule.h"
#endif

#ifdef MUSE_MODULE_WORKSPACE
#include "framework/workspace/workspacemodule.h"
#else
#include "framework/stubs/workspace/workspacestubmodule.h"
#endif

// Modules
#include "appshell/appshellmodule.h"

#ifdef MUSE_MODULE_AUTOBOT
#include "autobot/autobotmodule.h"
#endif

#ifdef MUE_BUILD_BRAILLE_MODULE
#include "braille/braillemodule.h"
#else
#include "stubs/braille/braillestubmodule.h"
#endif

#include "commonscene/commonscenemodule.h"
#include "context/contextmodule.h"

#ifdef MUE_BUILD_CONVERTER_MODULE
#include "converter/convertermodule.h"
#endif

#include "diagnostics/diagnosticsmodule.h"
#include "engraving/engravingmodule.h"

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
#include "importexport/musicxml/musicxmlmodule.h"
#include "importexport/bb/bbmodule.h"
#include "importexport/bww/bwwmodule.h"
#include "importexport/capella/capellamodule.h"
#include "importexport/guitarpro/guitarpromodule.h"
#include "importexport/midi/midimodule.h"
#include "importexport/musedata/musedatamodule.h"
#include "importexport/musx/musxmodule.h"
#include "importexport/ove/ovemodule.h"
#include "importexport/audioexport/audioexportmodule.h"
#include "importexport/imagesexport/imagesexportmodule.h"
#include "importexport/mei/meimodule.h"
#ifdef MUE_BUILD_VIDEOEXPORT_MODULE
#include "importexport/videoexport/videoexportmodule.h"
#endif
#else
#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
#include "importexport/imagesexport/imagesexportmodule.h"
#endif
#endif

#include "inspector/inspectormodule.h"

#ifdef MUE_BUILD_INSTRUMENTSSCENE_MODULE
#include "instrumentsscene/instrumentsscenemodule.h"
#else
#include "stubs/instrumentsscene/instrumentsscenestubmodule.h"
#endif

#ifdef MUE_BUILD_MUSESOUNDS_MODULE
#include "musesounds/musesoundsmodule.h"
#else
#include "stubs/musesounds/musesoundsstubmodule.h"
#endif

#ifdef MUE_BUILD_NOTATION_MODULE
#include "notation/notationmodule.h"
#else
#include "stubs/notation/notationstubmodule.h"
#endif

#ifdef MUE_BUILD_PALETTE_MODULE
#include "palette/palettemodule.h"
#else
#include "stubs/palette/palettestubmodule.h"
#endif

#ifdef MUE_BUILD_PLAYBACK_MODULE
#include "playback/playbackmodule.h"
#else
#include "stubs/playback/playbackstubmodule.h"
#endif

#ifdef MUSE_MODULE_EXTENSIONS
#include "extensions/extensionsmodule.h"
#endif

#include "print/printmodule.h"

#ifdef MUE_BUILD_PROJECT_MODULE
#include "project/projectmodule.h"
#else
#include "stubs/project/projectstubmodule.h"
#endif

#ifdef Q_OS_WASM
#include "wasmtest/wasmtestmodule.h"
#endif

using namespace muse;
using namespace mu::app;

std::shared_ptr<muse::IApplication> AppFactory::newApp(const CmdOptions& options) const
{
    if (options.runMode == IApplication::RunMode::GuiApp) {
        return newGuiApp(options);
    } else {
        return newConsoleApp(options);
    }
}

std::shared_ptr<muse::IApplication> AppFactory::newGuiApp(const CmdOptions& options) const
{
    modularity::ContextPtr ctx = std::make_shared<modularity::Context>();
    ++m_lastID;
    //ctx->id = m_lastID;
    ctx->id = -1; //! NOTE At the moment global ioc

    std::shared_ptr<GuiApp> app = std::make_shared<GuiApp>(options, ctx);

    //! NOTE `diagnostics` must be first, because it installs the crash handler.
    //! For other modules, the order is (an should be) unimportant.
    app->addModule(new muse::diagnostics::DiagnosticsModule());

    // framework
    app->addModule(new muse::accessibility::AccessibilityModule());
    app->addModule(new muse::actions::ActionsModule());
    app->addModule(new muse::audio::AudioModule());
#ifdef MUSE_MODULE_AUDIOPLUGINS
    app->addModule(new muse::audioplugins::AudioPluginsModule());
#endif
    app->addModule(new muse::draw::DrawModule());
    app->addModule(new muse::midi::MidiModule());
    app->addModule(new muse::mpe::MpeModule());

#ifdef MUSE_MODULE_MUSESAMPLER
    bool shouldAddMuseSamplerModule = true;
#ifndef MUSE_MODULE_MUSESAMPLER_LOAD_IN_DEBUG
    if (runtime::isDebug()) {
        shouldAddMuseSamplerModule = false;
        LOGI() << "Not adding MuseSampler module in a debug build";
    }
#endif

    if (shouldAddMuseSamplerModule) {
        app->addModule(new muse::musesampler::MuseSamplerModule());
    }
#endif

    app->addModule(new muse::network::NetworkModule());
    app->addModule(new muse::shortcuts::ShortcutsModule());
#ifdef MUSE_MODULE_UI
    app->addModule(new muse::ui::UiModule());
    app->addModule(new muse::uicomponents::UiComponentsModule());
    app->addModule(new muse::dock::DockModule());
#endif
    app->addModule(new muse::tours::ToursModule());
    app->addModule(new muse::vst::VSTModule());

// modules
#ifdef MUE_BUILD_APPSHELL_MODULE
    app->addModule(new mu::appshell::AppShellModule());
#endif

#ifdef MUSE_MODULE_AUTOBOT
    app->addModule(new muse::autobot::AutobotModule());
#endif

    app->addModule(new mu::braille::BrailleModule());

    app->addModule(new muse::cloud::CloudModule());
    app->addModule(new mu::commonscene::CommonSceneModule());
    app->addModule(new mu::context::ContextModule());

#ifdef MUE_BUILD_CONVERTER_MODULE
    app->addModule(new mu::converter::ConverterModule());
#endif

    app->addModule(new mu::engraving::EngravingModule());

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
    app->addModule(new mu::iex::bb::BBModule());
    app->addModule(new mu::iex::bww::BwwModule());
    app->addModule(new mu::iex::musicxml::MusicXmlModule());
    app->addModule(new mu::iex::capella::CapellaModule());
    app->addModule(new mu::iex::guitarpro::GuitarProModule());
    app->addModule(new mu::iex::midi::MidiModule());
    app->addModule(new mu::iex::musedata::MuseDataModule());
    app->addModule(new mu::iex::ove::OveModule());
    app->addModule(new mu::iex::audioexport::AudioExportModule());
    app->addModule(new mu::iex::imagesexport::ImagesExportModule());
    app->addModule(new mu::iex::mei::MeiModule());
#ifdef MUE_BUILD_VIDEOEXPORT_MODULE
    app->addModule(new mu::iex::videoexport::VideoExportModule());
#endif
#else
#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
    app->addModule(new mu::iex::imagesexport::ImagesExportModule());
#endif
#endif

    app->addModule(new mu::inspector::InspectorModule());
    app->addModule(new mu::instrumentsscene::InstrumentsSceneModule());
    app->addModule(new muse::languages::LanguagesModule());
    app->addModule(new muse::learn::LearnModule());
    app->addModule(new muse::mi::MultiInstancesModule());
    app->addModule(new mu::musesounds::MuseSoundsModule());
    app->addModule(new mu::notation::NotationModule());
    app->addModule(new mu::palette::PaletteModule());
    app->addModule(new mu::playback::PlaybackModule());
#ifdef MUSE_MODULE_EXTENSIONS
    app->addModule(new muse::extensions::ExtensionsModule());
#endif
    app->addModule(new mu::print::PrintModule());
    app->addModule(new mu::project::ProjectModule());
    app->addModule(new muse::update::UpdateModule());
    app->addModule(new muse::workspace::WorkspaceModule());

#ifdef Q_OS_WASM
    app->addModule(new mu::wasmtest::WasmTestModule());
#endif

    return app;
}

std::shared_ptr<muse::IApplication> AppFactory::newConsoleApp(const CmdOptions& options) const
{
    modularity::ContextPtr ctx = std::make_shared<modularity::Context>();
    ++m_lastID;
    // ctx->id = m_lastID;
    ctx->id = -1; //! NOTE At the moment global ioc

    std::shared_ptr<ConsoleApp> app = std::make_shared<ConsoleApp>(options, ctx);

    //! NOTE `diagnostics` must be first, because it installs the crash handler.
    //! For other modules, the order is (an should be) unimportant.
    app->addModule(new muse::diagnostics::DiagnosticsModule());

    //! TODO Some modules can be removed

    // framework
    app->addModule(new muse::accessibility::AccessibilityModule());
    app->addModule(new muse::actions::ActionsModule());
    app->addModule(new muse::audio::AudioModule());
#ifdef MUSE_MODULE_AUDIOPLUGINS
    app->addModule(new muse::audioplugins::AudioPluginsModule());
#endif
    app->addModule(new muse::draw::DrawModule());
    app->addModule(new muse::midi::MidiModule());
    app->addModule(new muse::mpe::MpeModule());

#ifdef MUSE_MODULE_MUSESAMPLER
    bool shouldAddMuseSamplerModule = true;
#ifndef MUSE_MODULE_MUSESAMPLER_LOAD_IN_DEBUG
    if (runtime::isDebug()) {
        shouldAddMuseSamplerModule = false;
        LOGI() << "Not adding MuseSampler module in a debug build";
    }
#endif

    if (shouldAddMuseSamplerModule) {
        app->addModule(new muse::musesampler::MuseSamplerModule());
    }
#endif

    app->addModule(new muse::network::NetworkModule());
    app->addModule(new muse::shortcuts::ShortcutsModule());
#ifdef MUSE_MODULE_UI
    app->addModule(new muse::ui::UiModule());
    app->addModule(new muse::uicomponents::UiComponentsModule());
    app->addModule(new muse::dock::DockModule());
#endif
    app->addModule(new muse::tours::ToursModule());
    app->addModule(new muse::vst::VSTModule());

// modules
#ifdef MUE_BUILD_APPSHELL_MODULE
    app->addModule(new mu::appshell::AppShellModule());
#endif

#ifdef MUSE_MODULE_AUTOBOT
    app->addModule(new muse::autobot::AutobotModule());
#endif

    app->addModule(new mu::braille::BrailleModule());

    app->addModule(new muse::cloud::CloudModule());
    app->addModule(new mu::commonscene::CommonSceneModule());
    app->addModule(new mu::context::ContextModule());

#ifdef MUE_BUILD_CONVERTER_MODULE
    app->addModule(new mu::converter::ConverterModule());
#endif

    app->addModule(new mu::engraving::EngravingModule());

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
    app->addModule(new mu::iex::bb::BBModule());
    app->addModule(new mu::iex::bww::BwwModule());
    app->addModule(new mu::iex::musicxml::MusicXmlModule());
    app->addModule(new mu::iex::capella::CapellaModule());
    app->addModule(new mu::iex::guitarpro::GuitarProModule());
    app->addModule(new mu::iex::midi::MidiModule());
    app->addModule(new mu::iex::musedata::MuseDataModule());
    app->addModule(new mu::iex::musx::MusxModule());
    app->addModule(new mu::iex::ove::OveModule());
    app->addModule(new mu::iex::audioexport::AudioExportModule());
    app->addModule(new mu::iex::imagesexport::ImagesExportModule());
    app->addModule(new mu::iex::mei::MeiModule());
#ifdef MUE_BUILD_VIDEOEXPORT_MODULE
    app->addModule(new mu::iex::videoexport::VideoExportModule());
#endif
#else
#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
    app->addModule(new mu::iex::imagesexport::ImagesExportModule());
#endif
#endif

    app->addModule(new mu::inspector::InspectorModule());
    app->addModule(new mu::instrumentsscene::InstrumentsSceneModule());
    app->addModule(new muse::languages::LanguagesModule());
    app->addModule(new muse::learn::LearnModule());
    app->addModule(new muse::mi::MultiInstancesModule());
    app->addModule(new mu::notation::NotationModule());
    app->addModule(new mu::palette::PaletteModule());
    app->addModule(new mu::playback::PlaybackModule());
#ifdef MUSE_MODULE_EXTENSIONS
    app->addModule(new muse::extensions::ExtensionsModule());
#endif
    app->addModule(new mu::print::PrintModule());
    app->addModule(new mu::project::ProjectModule());
    app->addModule(new muse::update::UpdateModule());
    app->addModule(new muse::workspace::WorkspaceModule());

#ifdef Q_OS_WASM
    app->addModule(new mu::wasmtest::WasmTestModule());
#endif

    return app;
}
