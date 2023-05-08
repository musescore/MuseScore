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

#include "runtime.h"
#include "log.h"

#include "app.h"

#include "framework/global/globalmodule.h"

#ifdef MUE_BUILD_ACCESSIBILITY_MODULE
#include "framework/accessibility/accessibilitymodule.h"
#else
#include "stubs/framework/accessibility/accessibilitystubmodule.h"
#endif

#include "framework/actions/actionsmodule.h"

#ifdef MUE_BUILD_AUDIO_MODULE
#include "framework/audio/audiomodule.h"
#else
#include "stubs/framework/audio/audiostubmodule.h"
#endif

#include "framework/draw/drawmodule.h"
#include "framework/fonts/fontsmodule.h"

#ifdef MUE_BUILD_MIDI_MODULE
#include "framework/midi/midimodule.h"
#else
#include "stubs/framework/midi/midistubmodule.h"
#endif

#ifdef MUE_BUILD_MIDI_MODULE
#include "framework/mpe/mpemodule.h"
#else
#include "stubs/framework/mpe/mpestubmodule.h"
#endif

#ifdef MUE_BUILD_MUSESAMPLER_MODULE
#include "framework/musesampler/musesamplermodule.h"
#endif

#ifdef MUE_BUILD_NETWORK_MODULE
#include "framework/network/networkmodule.h"
#else
#include "stubs/framework/network/networkstubmodule.h"
#endif

#ifdef MUE_BUILD_SHORTCUTS_MODULE
#include "framework/shortcuts/shortcutsmodule.h"
#else
#include "stubs/framework/shortcuts/shortcutsstubmodule.h"
#endif

#ifdef MUE_BUILD_UI_MODULE
#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"
#endif

#ifdef MUE_BUILD_VST_MODULE
#include "framework/vst/vstmodule.h"
#else
#include "stubs/framework/vst/vststubmodule.h"
#endif

// Modules
#include "appshell/appshellmodule.h"

#ifdef MUE_BUILD_AUTOBOT_MODULE
#include "autobot/autobotmodule.h"
#endif

#ifdef MUE_BUILD_CLOUD_MODULE
#include "cloud/cloudmodule.h"
#else
#include "stubs/cloud/cloudstubmodule.h"
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
#include "importexport/braille/braillemodule.h"
#include "importexport/bww/bwwmodule.h"
#include "importexport/capella/capellamodule.h"
#include "importexport/guitarpro/guitarpromodule.h"
#include "importexport/midi/midimodule.h"
#include "importexport/musedata/musedatamodule.h"
#include "importexport/ove/ovemodule.h"
#include "importexport/audioexport/audioexportmodule.h"
#include "importexport/imagesexport/imagesexportmodule.h"
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

#ifdef MUE_BUILD_LANGUAGES_MODULE
#include "languages/languagesmodule.h"
#else
#include "stubs/languages/languagesstubmodule.h"
#endif

#ifdef MUE_BUILD_LEARN_MODULE
#include "learn/learnmodule.h"
#else
#include "stubs/learn/learnmodule.h"
#endif

#ifdef MUE_BUILD_MULTIINSTANCES_MODULE
#include "multiinstances/multiinstancesmodule.h"
#else
#include "stubs/multiinstances/multiinstancesstubmodule.h"
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

#ifdef MUE_BUILD_PLUGINS_MODULE
#include "plugins/pluginsmodule.h"
#else
#include "stubs/plugins/pluginsstubmodule.h"
#endif

#include "print/printmodule.h"

#ifdef MUE_BUILD_PROJECT_MODULE
#include "project/projectmodule.h"
#else
#include "stubs/project/projectstubmodule.h"
#endif

#ifdef MUE_BUILD_UPDATE_MODULE
#include "update/updatemodule.h"
#else
#include "stubs/update/updatestubmodule.h"
#endif

#ifdef MUE_BUILD_WORKSPACE_MODULE
#include "workspace/workspacemodule.h"
#else
#include "stubs/workspace/workspacestubmodule.h"
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

#include <iostream>

int main(int argc, char** argv)
{
    // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC under Windows, which
    // would otherwise default to the local ANSI code page and cause corruption of any non-ANSI Unicode characters in command-line arguments.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    mu::app::App app;

    //! NOTE `diagnostics` must be first, because it installs the crash handler.
    //! For other modules, the order is (an should be) unimportant.
    app.addModule(new mu::diagnostics::DiagnosticsModule());

    // framework
    app.addModule(new mu::accessibility::AccessibilityModule());
    app.addModule(new mu::actions::ActionsModule());
    app.addModule(new mu::audio::AudioModule());
    app.addModule(new mu::draw::DrawModule());
    app.addModule(new mu::fonts::FontsModule());
    app.addModule(new mu::midi::MidiModule());
    app.addModule(new mu::mpe::MpeModule());
#ifdef MUE_BUILD_MUSESAMPLER_MODULE
    app.addModule(new mu::musesampler::MuseSamplerModule());
#endif
    app.addModule(new mu::network::NetworkModule());
    app.addModule(new mu::shortcuts::ShortcutsModule());
#ifdef MUE_BUILD_UI_MODULE
    app.addModule(new mu::ui::UiModule());
    app.addModule(new mu::uicomponents::UiComponentsModule());
#endif
    app.addModule(new mu::vst::VSTModule());

    // modules
#ifdef MUE_BUILD_APPSHELL_MODULE
    app.addModule(new mu::appshell::AppShellModule());
#endif

#ifdef MUE_BUILD_AUTOBOT_MODULE
    app.addModule(new mu::autobot::AutobotModule());
#endif

    app.addModule(new mu::cloud::CloudModule());
    app.addModule(new mu::commonscene::CommonSceneModule());
    app.addModule(new mu::context::ContextModule());

#ifdef MUE_BUILD_CONVERTER_MODULE
    app.addModule(new mu::converter::ConverterModule());
#endif

    app.addModule(new mu::engraving::EngravingModule());

#ifdef MUE_BUILD_IMPORTEXPORT_MODULE
    app.addModule(new mu::iex::bb::BBModule());
    app.addModule(new mu::iex::braille::BrailleModule());
    app.addModule(new mu::iex::bww::BwwModule());
    app.addModule(new mu::iex::musicxml::MusicXmlModule());
    app.addModule(new mu::iex::capella::CapellaModule());
    app.addModule(new mu::iex::guitarpro::GuitarProModule());
    app.addModule(new mu::iex::midi::MidiModule());
    app.addModule(new mu::iex::musedata::MuseDataModule());
    app.addModule(new mu::iex::ove::OveModule());
    app.addModule(new mu::iex::audioexport::AudioExportModule());
    app.addModule(new mu::iex::imagesexport::ImagesExportModule());
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
    app.addModule(new mu::languages::LanguagesModule());
    app.addModule(new mu::learn::LearnModule());
    app.addModule(new mu::mi::MultiInstancesModule());
    app.addModule(new mu::notation::NotationModule());
    app.addModule(new mu::palette::PaletteModule());
    app.addModule(new mu::playback::PlaybackModule());
    app.addModule(new mu::plugins::PluginsModule());
    app.addModule(new mu::print::PrintModule());
    app.addModule(new mu::project::ProjectModule());
    app.addModule(new mu::update::UpdateModule());
    app.addModule(new mu::workspace::WorkspaceModule());

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
