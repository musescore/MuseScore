/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

namespace mu::project {
inline static const muse::rcommand::Command PROJECT_NEW_COMMAND("command://project/new");
inline static const muse::rcommand::Command PROJECT_OPEN_COMMAND("command://project/open");
inline static const muse::rcommand::Command PROJECT_CLOSE_COMMAND("command://project/close");

inline static const muse::rcommand::Command PROJECT_SAVE_COMMAND("command://project/save");
inline static const muse::rcommand::Command PROJECT_SAVE_AS_COMMAND("command://project/save-as");
inline static const muse::rcommand::Command PROJECT_SAVE_A_COPY_COMMAND("command://project/save-a-copy");
inline static const muse::rcommand::Command PROJECT_SAVE_SELECTION_COMMAND("command://project/save-selection");
inline static const muse::rcommand::Command PROJECT_SAVE_TO_CLOUD_COMMAND("command://project/save-to-cloud");
inline static const muse::rcommand::Command PROJECT_SAVE_AT_COMMAND("command://project/save-at");

inline static const muse::rcommand::Command PROJECT_PUBLISH_COMMAND("command://project/publish");
inline static const muse::rcommand::Command PROJECT_SHARED_AUDIO_COMMAND("command://project/shared-audio");

inline static const muse::rcommand::Command PROJECT_EXPORT_COMMAND("command://project/export");
inline static const muse::rcommand::Command PROJECT_IMPORT_PDF_COMMAND("command://project/import-pdf");
inline static const muse::rcommand::Command PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND("command://project/import-audio-to-score");

inline static const muse::rcommand::Command PROJECT_PRINT_COMMAND("command://project/print");
inline static const muse::rcommand::Command PROJECT_CLEAR_RECENT_COMMAND("command://project/clear-recent");
inline static const muse::rcommand::Command PROJECT_CONTINUE_LAST_SESSION_COMMAND("command://project/continue-last-session");
inline static const muse::rcommand::Command PROJECT_PROPERTIES_COMMAND("command://project/properties");
}
