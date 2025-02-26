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
#ifndef MU_CONTEXT_UICONTEXT_H
#define MU_CONTEXT_UICONTEXT_H

#include "ui/uiaction.h"

namespace mu::context {
//! NOTE Determines where to be, what the user is doing

// common ui (re declared for convenience)
static constexpr muse::ui::UiContext UiCtxUnknown = muse::ui::UiCtxUnknown;
static constexpr muse::ui::UiContext UiCtxAny = muse::ui::UiCtxAny;

static constexpr muse::ui::UiContext UiCtxHomeOpened = muse::ui::UiCtxHomeOpened;
static constexpr muse::ui::UiContext UiCtxProjectOpened = muse::ui::UiCtxProjectOpened;
static constexpr muse::ui::UiContext UiCtxProjectFocused = muse::ui::UiCtxProjectFocused;

// application-specific contexts
static constexpr muse::ui::UiContext UiCtxPublishOpened = "UiCtxPublishOpened";
static constexpr muse::ui::UiContext UiCtxDevToolsOpened = "UiCtxDevToolsOpened";
}

#endif // MU_CONTEXT_UICONTEXT_H
