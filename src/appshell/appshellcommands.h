/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

 #pragma once

 #include "rcommand/commandtypes.h"

namespace mu::appshell {
inline static const muse::rcommand::Command APP_QUIT_COMMAND("command://app/quit");
inline static const muse::rcommand::Command APP_RESTART_COMMAND("command://app/restart");
inline static const muse::rcommand::Command APP_FULLSCREEN_COMMAND("command://app/fullscreen");
inline static const muse::rcommand::Command APP_ABOUT_MUSESCORE_COMMAND("command://app/about-musescore");
inline static const muse::rcommand::Command APP_ABOUT_QT_COMMAND("command://app/about-qt");
inline static const muse::rcommand::Command APP_ABOUT_MUSICXML_COMMAND("command://app/about-musicxml");
inline static const muse::rcommand::Command APP_ONLINE_HANDBOOK_COMMAND("command://app/online-handbook");
inline static const muse::rcommand::Command APP_ASK_HELP_COMMAND("command://app/ask-help");
inline static const muse::rcommand::Command APP_ACCESSIBILITY_STATEMENT_COMMAND("command://app/accessibility-statement");
inline static const muse::rcommand::Command APP_PREFERENCES_COMMAND("command://app/preferences");
inline static const muse::rcommand::Command APP_REVERT_TO_FACTORY_COMMAND("command://app/revert-to-factory");
inline static const muse::rcommand::Command APP_EXTENSIONS_COMMAND("command://app/extensions");
}
