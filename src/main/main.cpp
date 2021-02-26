//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include <QTextCodec>

#include "config.h"
#include "runtime.h"
#include "log.h"

#include "appshell/appshell.h"

#include "framework/global/globalmodule.h"
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#include "framework/fonts/fontsmodule.h"
#include "framework/actions/actionsmodule.h"
#ifdef BUILD_SHORTCUTS_MODULE
#include "framework/shortcuts/shortcutsmodule.h"
#else
#include "stubs/framework/shortcuts/shortcutsstubmodule.h"
#endif

#ifdef BUILD_SYSTEM_MODULE
#include "framework/system/systemmodule.h"
#else
#include "stubs/framework/system/systemstubmodule.h"
#endif
#ifdef BUILD_NETWORK_MODULE
#include "framework/network/networkmodule.h"
#else
#include "stubs/framework/network/networkstubmodule.h"
#endif

#ifdef BUILD_AUDIO_MODULE
#include "framework/audio/audiomodule.h"
#else
#include "stubs/framework/audio/audiostubmodule.h"
#endif
#include "framework/midi/midimodule.h"

#include "appshell/appshellmodule.h"
#include "context/contextmodule.h"
#ifdef BUILD_USERSCORES_MODULE
#include "userscores/userscoresmodule.h"
#else
#include "stubs/userscores/userscoresstubmodule.h"
#endif
#include "notation/notationmodule.h"

#include "importexport/musicxml/musicxmlmodule.h"
#include "importexport/bb/bbmodule.h"
#include "importexport/bww/bwwmodule.h"
#include "importexport/capella/capellamodule.h"
#include "importexport/guitarpro/guitarpromodule.h"
#include "importexport/midiimport/midiimportmodule.h"
#include "importexport/ove/ovemodule.h"
#include "importexport/audioexport/audioexportmodule.h"
#include "importexport/imagesexport/imagesexportmodule.h"

#include "commonscene/commonscenemodule.h"
#ifdef BUILD_PALETTE_MODULE
#include "palette/palettemodule.h"
#else
#include "stubs/palette/palettestubmodule.h"
#endif
#include "inspector/inspectormodule.h"
#ifdef BUILD_PLAYBACK_MODULE
#include "playback/playbackmodule.h"
#else
#include "stubs/playback/playbackstubmodule.h"
#endif
#ifdef BUILD_INSTRUMENTS_MODULE
#include "instruments/instrumentsmodule.h"
#else
#include "stubs/instruments/instrumentsstubmodule.h"
#endif
#include "converter/convertermodule.h"

#ifdef BUILD_VST
#include "framework/vst/vstmodule.h"
#endif
#ifdef BUILD_TELEMETRY_MODULE
#include "framework/telemetry/telemetrymodule.h"
#endif

#ifndef Q_OS_WASM
#ifdef BUILD_WORKSPACE_MODULE
#include "workspace/workspacemodule.h"
#else
#include "stubs/workspace/workspacestubmodule.h"
#endif
#ifdef BUILD_PLUGINS_MODULE
#include "plugins/pluginsmodule.h"
#else
#include "stubs/plugins/pluginsstubmodule.h"
#endif

#ifdef BUILD_CLOUD_MODULE
#include "cloud/cloudmodule.h"
#else
#include "stubs/cloud/cloudstubmodule.h"
#endif

#ifdef BUILD_EXTENSIONS_MODULE
#include "extensions/extensionsmodule.h"
#else
#include "stubs/extensions/extensionsstubmodule.h"
#endif

#ifdef BUILD_LANGUAGES_MODULE
#include "languages/languagesmodule.h"
#else
#include "stubs/languages/languagesstubmodule.h"
#endif

#include "autobot/autobotmodule.h"

#else
#include "wasmtest/wasmtestmodule.h"
#endif

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <vector>
#include <algorithm>
#include <windows.h>
#include <shellapi.h>
#endif

#include <iostream>

int main(int argc, char** argv)
{
    // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC under Windows, which
    // would otherwise default to the local ANSI code page and cause corruption of any non-ANSI Unicode characters in command-line arguments.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    mu::appshell::AppShell app;

    //! NOTE `telemetry` must be first, because it install crash handler.
    //! others modules order not important (must be)
#ifdef BUILD_TELEMETRY_MODULE
    app.addModule(new mu::telemetry::TelemetryModule());
#endif
    app.addModule(new mu::fonts::FontsModule());
    app.addModule(new mu::ui::UiModule());
    app.addModule(new mu::uicomponents::UiComponentsModule());
#ifdef BUILD_SYSTEM_MODULE
    app.addModule(new mu::system::SystemModule());
#else
    app.addModule(new mu::system::SystemStubModule());
#endif

#ifdef BUILD_NETWORK_MODULE
    app.addModule(new mu::network::NetworkModule());
#else
    app.addModule(new mu::network::NetworkStubModule());
#endif

    app.addModule(new mu::actions::ActionsModule());
    app.addModule(new mu::appshell::AppShellModule());

    app.addModule(new mu::context::ContextModule());
#ifdef BUILD_SHORTCUTS_MODULE
    app.addModule(new mu::shortcuts::ShortcutsModule());
#else
    app.addModule(new mu::shortcuts::ShortcutsStubModule());
#endif

#ifdef BUILD_AUDIO_MODULE
    app.addModule(new mu::audio::AudioModule());
#else
    app.addModule(new mu::audio::AudioStubModule());
#endif
    app.addModule(new mu::midi::MidiModule());

#ifdef BUILD_USERSCORES_MODULE
    app.addModule(new mu::userscores::UserScoresModule());
#else
    app.addModule(new mu::userscores::UserScoresStubModule());
#endif

    app.addModule(new mu::notation::NotationModule());
    app.addModule(new mu::commonscene::CommonSceneModule());
#ifdef BUILD_PLAYBACK_MODULE
    app.addModule(new mu::playback::PlaybackModule());
#else
    app.addModule(new mu::playback::PlaybackStubModule());
#endif

#ifdef BUILD_INSTRUMENTS_MODULE
    app.addModule(new mu::instruments::InstrumentsModule());
#else
    app.addModule(new mu::instruments::InstrumentsStubModule());
#endif
#ifdef BUILD_VST
    app.addModule(new mu::vst::VSTModule());
#endif

    app.addModule(new mu::inspector::InspectorModule());
#ifdef BUILD_PALETTE_MODULE
    app.addModule(new mu::palette::PaletteModule());
#else
    app.addModule(new mu::palette::PaletteStubModule());
#endif
    app.addModule(new mu::converter::ConverterModule());

#ifndef Q_OS_WASM
    app.addModule(new mu::iex::bb::BBModule());
    app.addModule(new mu::iex::bww::BwwModule());
    app.addModule(new mu::iex::musicxml::MusicXmlModule());
    app.addModule(new mu::iex::capella::CapellaModule());
    app.addModule(new mu::iex::guitarpro::GuitarProModule());
    app.addModule(new mu::iex::midiimport::MidiImportModule());
    app.addModule(new mu::iex::ove::OveModule());
    app.addModule(new mu::iex::audioexport::AudioExportModule());
    app.addModule(new mu::iex::imagesexport::ImagesExportModule());

#ifdef BUILD_WORKSPACE_MODULE
    app.addModule(new mu::workspace::WorkspaceModule());
#else
    app.addModule(new mu::workspace::WorkspaceStubModule());
#endif
#ifdef BUILD_PLUGINS_MODULE
    app.addModule(new mu::plugins::PluginsModule());
#else
    app.addModule(new mu::plugins::PluginsStubModule());
#endif
#ifdef BUILD_CLOUD_MODULE
    app.addModule(new mu::cloud::CloudModule());
#else
    app.addModule(new mu::cloud::CloudStubModule());
#endif
#ifdef BUILD_EXTENSIONS_MODULE
    app.addModule(new mu::extensions::ExtensionsModule());
#else
    app.addModule(new mu::extensions::ExtensionsStubModule());
#endif
#ifdef BUILD_LANGUAGES_MODULE
    app.addModule(new mu::languages::LanguagesModule());
#else
    app.addModule(new mu::languages::LanguagesStubModule());
#endif

    app.addModule(new mu::autobot::AutobotModule());

#else
    app.addModule(new mu::wasmtest::WasmTestModule());
#endif

#if (defined (_MSCVER) || defined (_MSC_VER))
    // On MSVC under Windows, we need to manually retrieve the command-line arguments and convert them from UTF-16 to UTF-8.
    // This prevents data loss if there are any characters that wouldn't fit in the local ANSI code page.
    int argcUTF16 = 0;
    LPWSTR* argvUTF16 = CommandLineToArgvW(GetCommandLineW(), &argcUTF16);

    std::vector<QByteArray> argvUTF8Q;
    std::for_each(argvUTF16, argvUTF16 + argcUTF16, [&argvUTF8Q](const auto& arg) {
        argvUTF8Q.emplace_back(QString::fromUtf16(reinterpret_cast<const char16_t*>(arg), -1).toUtf8());
    });

    LocalFree(argvUTF16);

    std::vector<char*> argvUTF8;
    for (auto& arg : argvUTF8Q) {
        argvUTF8.push_back(arg.data());
    }

    // Don't use the arguments passed to main(), because they're in the local ANSI code page.
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    int argcFinal = argcUTF16;
    char** argvFinal = argvUTF8.data();
#else

    int argcFinal = argc;
    char** argvFinal = argv;

#endif

    int code = app.run(argcFinal, argvFinal);
    LOGI() << "Good buy!! code: " << code;
    return code;
}
