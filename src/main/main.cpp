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
#include "framework/shortcuts/shortcutsmodule.h"
#include "framework/workspace/workspacemodule.h"
#include "framework/system/systemmodule.h"
#include "framework/network/networkmodule.h"
#include "framework/audio/audiomodule.h"
#include "framework/rpc/rpcmodule.h"
#include "framework/midi/midimodule.h"

#include "appshell/appshellmodule.h"
#include "cloud/cloudmodule.h"
#include "context/contextmodule.h"
#include "userscores/userscoresmodule.h"
#include "extensions/extensionsmodule.h"
#include "languages/languagesmodule.h"
#include "plugins/pluginsmodule.h"
#include "notation/notationmodule.h"
#include "importexport/importexportmodule.h"
#include "importexport/importexportmodule.h"
#include "commonscene/commonscenemodule.h"
#include "palette/palettemodule.h"
#include "inspector/inspectormodule.h"
#include "playback/playbackmodule.h"
#include "instruments/instrumentsmodule.h"
#include "converter/convertermodule.h"

#ifdef BUILD_VST
#include "framework/vst/vstmodule.h"
#endif
#ifdef BUILD_TELEMETRY_MODULE
#include "framework/telemetry/telemetrymodule.h"
#endif

#ifndef Q_OS_WASM
#include "framework/workspace/workspacemodule.h"
#include "plugins/pluginsmodule.h"
#include "importexport/importexportmodule.h"
#include "cloud/cloudmodule.h"
#include "extensions/extensionsmodule.h"
#include "languages/languagesmodule.h"
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
    app.addModule(new mu::framework::UiModule());
    app.addModule(new mu::framework::UiComponentsModule());
    app.addModule(new mu::framework::SystemModule());
    app.addModule(new mu::framework::NetworkModule());

    app.addModule(new mu::actions::ActionsModule());
    app.addModule(new mu::appshell::AppShellModule());

    app.addModule(new mu::context::ContextModule());
    app.addModule(new mu::shortcuts::ShortcutsModule());

    app.addModule(new mu::audio::AudioModule());
    app.addModule(new mu::rpc::RpcModule());
    app.addModule(new mu::midi::MidiModule());
    app.addModule(new mu::userscores::UserScoresModule());

    app.addModule(new mu::notation::NotationModule());
    app.addModule(new mu::commonscene::CommonSceneModule());
    app.addModule(new mu::playback::PlaybackModule());
    app.addModule(new mu::instruments::InstrumentsModule());
#ifdef BUILD_VST
    app.addModule(new mu::vst::VSTModule());
#endif

    app.addModule(new mu::inspector::InspectorModule());
    app.addModule(new mu::palette::PaletteModule());
    app.addModule(new mu::converter::ConverterModule());

#ifndef Q_OS_WASM
    app.addModule(new mu::importexport::ImportExportModule());
    app.addModule(new mu::workspace::WorkspaceModule());
    app.addModule(new mu::plugins::PluginsModule());
    app.addModule(new mu::cloud::CloudModule());
    app.addModule(new mu::extensions::ExtensionsModule());
    app.addModule(new mu::languages::LanguagesModule());
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
