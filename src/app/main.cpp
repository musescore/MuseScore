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

#include <QTextCodec>

#include <csignal>

#include "runtime.h"
#include "log.h"

#include "app.h"

#include "framework/global/globalmodule.h"

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

#ifdef MUSE_MODULE_UI
#include "framework/dockwindow/dockmodule.h"
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#endif

#ifdef MUE_BUILD_UPDATE_MODULE
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

#ifdef MUSE_MODULE_WORKSPACE
#include "workspacescene/workspacescenemodule.h"
#else
#include "stubs/workspacescene/workspacescenestubmodule.h"
#endif

#ifdef Q_OS_WASM
#include "wasmtest/wasmtestmodule.h"
#endif

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <vector>
#include <algorithm>
#include <windows.h>
#include <shellapi.h>
#endif

#ifndef MUE_BUILD_CRASHPAD_CLIENT
static void crashCallback(int signum)
{
    const char* signame = "UNKNOWN SIGNAME";
    const char* sigdescript = "";
    switch (signum) {
    case SIGILL:
        signame = "SIGILL";
        sigdescript = "Illegal Instruction";
        break;
    case SIGSEGV:
        signame = "SIGSEGV";
        sigdescript =  "Invalid memory reference";
        break;
    }
    LOGE() << "Oops! Application crashed with signal: [" << signum << "] " << signame << "-" << sigdescript;
    exit(EXIT_FAILURE);
}

#endif

int main(int argc, char** argv)
{
#ifndef MUE_BUILD_CRASHPAD_CLIENT
    signal(SIGSEGV, crashCallback);
    signal(SIGILL, crashCallback);
    signal(SIGFPE, crashCallback);
#endif

    // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC under Windows, which
    // would otherwise default to the local ANSI code page and cause corruption of any non-ANSI Unicode characters in command-line arguments.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    mu::app::App app;

    //! NOTE `diagnostics` must be first, because it installs the crash handler.
    //! For other modules, the order is (an should be) unimportant.
    app.addModule(new mu::diagnostics::DiagnosticsModule());

    // framework
    app.addModule(new muse::accessibility::AccessibilityModule());
    app.addModule(new muse::actions::ActionsModule());
    app.addModule(new muse::audio::AudioModule());
    app.addModule(new muse::draw::DrawModule());
    app.addModule(new muse::midi::MidiModule());
    app.addModule(new mu::mpe::MpeModule());
#ifdef MUSE_MODULE_MUSESAMPLER
    app.addModule(new muse::musesampler::MuseSamplerModule());
#endif
    app.addModule(new muse::network::NetworkModule());
    app.addModule(new muse::shortcuts::ShortcutsModule());
#ifdef MUSE_MODULE_UI
    app.addModule(new muse::ui::UiModule());
    app.addModule(new muse::uicomponents::UiComponentsModule());
    app.addModule(new muse::dock::DockModule());
#endif
    app.addModule(new muse::vst::VSTModule());

    // modules
#ifdef MUE_BUILD_APPSHELL_MODULE
    app.addModule(new mu::appshell::AppShellModule());
#endif

#ifdef MUSE_MODULE_AUTOBOT
    app.addModule(new muse::autobot::AutobotModule());
#endif

    app.addModule(new mu::braille::BrailleModule());

    app.addModule(new muse::cloud::CloudModule());
    app.addModule(new mu::commonscene::CommonSceneModule());
    app.addModule(new mu::context::ContextModule());

#ifdef MUE_BUILD_CONVERTER_MODULE
    app.addModule(new mu::converter::ConverterModule());
#endif

    app.addModule(new mu::engraving::EngravingModule());

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
    app.addModule(new mu::iex::bb::BBModule());
    app.addModule(new mu::iex::bww::BwwModule());
    app.addModule(new mu::iex::musicxml::MusicXmlModule());
    app.addModule(new mu::iex::capella::CapellaModule());
    app.addModule(new mu::iex::guitarpro::GuitarProModule());
    app.addModule(new mu::iex::midi::MidiModule());
    app.addModule(new mu::iex::musedata::MuseDataModule());
    app.addModule(new mu::iex::ove::OveModule());
    app.addModule(new mu::iex::audioexport::AudioExportModule());
    app.addModule(new mu::iex::imagesexport::ImagesExportModule());
    app.addModule(new mu::iex::mei::MeiModule());
#ifdef MUE_BUILD_VIDEOEXPORT_MODULE
    app.addModule(new mu::iex::videoexport::VideoExportModule());
#endif
#else
#ifdef MUE_BUILD_IMAGESEXPORT_MODULE
    app.addModule(new mu::iex::imagesexport::ImagesExportModule());
#endif
#endif

    app.addModule(new mu::inspector::InspectorModule());
    app.addModule(new mu::instrumentsscene::InstrumentsSceneModule());
    app.addModule(new muse::languages::LanguagesModule());
    app.addModule(new muse::learn::LearnModule());
    app.addModule(new mu::mi::MultiInstancesModule());
    app.addModule(new mu::notation::NotationModule());
    app.addModule(new mu::palette::PaletteModule());
    app.addModule(new mu::playback::PlaybackModule());
#ifdef MUSE_MODULE_EXTENSIONS
    app.addModule(new muse::extensions::ExtensionsModule());
#endif
    app.addModule(new mu::print::PrintModule());
    app.addModule(new mu::project::ProjectModule());
    app.addModule(new mu::update::UpdateModule());
    app.addModule(new muse::workspace::WorkspaceModule());
    app.addModule(new mu::workspacescene::WorkspaceSceneModule());

#ifdef Q_OS_WASM
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
    LOGI() << "Goodbye!! code: " << code;
    return code;
}
