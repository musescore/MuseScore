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
#include "uicontextresolver.h"

#include "diagnostics/diagnosticutils.h"

#include "shortcutcontext.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace mu::context;
using namespace muse;
using namespace muse::ui;

static const muse::Uri HOME_PAGE_URI("musescore://home");
static const muse::Uri NOTATION_PAGE_URI("musescore://notation");
static const muse::Uri PUBLISH_PAGE_URI("musescore://publish");
static const muse::Uri DEVTOOLS_PAGE_URI("musescore://devtools");

static const muse::Uri EXTENSIONS_DIALOG_URI("muse://extensions/viewer");

static const QString NOTATION_NAVIGATION_PANEL("ScoreView");

void UiContextResolver::init()
{
    interactive()->currentUri().ch.onReceive(this, [this](const Uri&) {
        notifyAboutContextChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        notifyAboutContextChanged();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto notation = globalContext()->currentNotation();
        if (notation) {
            notation->interaction()->selectionChanged().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });

            notation->interaction()->textEditingStarted().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });

            notation->interaction()->textEditingEnded().onReceive(this, [this](engraving::TextBase*) {
                notifyAboutContextChanged();
            });

            notation->undoStack()->stackChanged().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });

            notation->interaction()->noteInput()->noteInputStarted().onReceive(this, [this](bool) {
                notifyAboutContextChanged();
            });

            notation->interaction()->noteInput()->noteInputEnded().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });
        }
        notifyAboutContextChanged();
    });

    navigationController()->navigationChanged().onNotify(this, [this]() {
        notifyAboutContextChanged();
    });
}

void UiContextResolver::notifyAboutContextChanged()
{
    m_currentUiContextChanged.notify();
}

UiContext UiContextResolver::currentUiContext() const
{
    TRACEFUNC;
    Uri currentUri = interactive()->currentUri().val;

#ifdef MUSE_MODULE_DIAGNOSTICS
    currentUri = diagnostics::diagnosticCurrentUri(interactive()->stack());
#endif

    if (currentUri == HOME_PAGE_URI) {
        return context::UiCtxHomeOpened;
    }

    if (currentUri == NOTATION_PAGE_URI) {
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            //! NOTE The notation page is open, but the notation itself is not loaded - we consider that the notation is not open.
            //! We need to think, maybe we need a separate value for this case.
            return context::UiCtxUnknown;
        }

        INavigationPanel* activePanel = navigationController()->activePanel();
        if (activePanel) {
            if (activePanel->name() == NOTATION_NAVIGATION_PANEL) {
                return context::UiCtxProjectFocused;
            }
        }

        return context::UiCtxProjectOpened;
    }

    if (currentUri == PUBLISH_PAGE_URI) {
        return context::UiCtxPublishOpened;
    }

    if (currentUri == DEVTOOLS_PAGE_URI) {
        return context::UiCtxDevToolsOpened;
    }

    if (interactive()->isCurrentUriDialog().val) {
        bool isExtensionDialog = currentUri == EXTENSIONS_DIALOG_URI;
        if (!isExtensionDialog) {
            return context::UiCtxDialogOpened;
        }
    }

    return context::UiCtxUnknown;
}

bool UiContextResolver::match(const muse::ui::UiContext& currentCtx, const muse::ui::UiContext& actCtx) const
{
    if (actCtx == context::UiCtxAny) {
        return true;
    }

    //! NOTE: Context could be unknown if a plugin is currently open, in which case we should return true under
    //! the following circumstances (see issue #24673)...
    if ((currentCtx == context::UiCtxProjectFocused || currentCtx == context::UiCtxUnknown)
        && actCtx == context::UiCtxProjectOpened && globalContext()->currentNotation()) {
        return true;
    }

    return currentCtx == actCtx;
}

bool UiContextResolver::matchWithCurrent(const UiContext& ctx) const
{
    if (ctx == muse::ui::UiCtxAny) {
        return true;
    }

    UiContext currentCtx = currentUiContext();
    return match(currentCtx, ctx);
}

muse::async::Notification UiContextResolver::currentUiContextChanged() const
{
    return m_currentUiContextChanged;
}

bool UiContextResolver::isShortcutContextAllowed(const std::string& scContext) const
{
    //! NOTE If (when) there are many different contexts here,
    //! then the implementation of this method will need to be changed
    //! so that it does not become spaghetti-code.
    //! It would be nice if this context as part of the UI context,
    //! for this we should complicate the implementation of the UI context,
    //! probably make a tree, for example:
    //! NotationOpened
    //!     NotationFocused
    //!         NotationStaffTab
    //!
    //! UPDATE I'm now adding one more context for list VS range selection, but this is
    //! quite clearly not an optimal solution. In future, we need a general system to
    //! allow/disallow shortcuts based on any property of the currentNotation. [M.S.]

    if (CTX_NOTATION_OPENED == scContext) {
        return matchWithCurrent(context::UiCtxProjectOpened);
    } else if (CTX_NOTATION_FOCUSED == scContext) {
        return matchWithCurrent(context::UiCtxProjectFocused);
    } else if (CTX_NOT_NOTATION_FOCUSED == scContext) {
        return !matchWithCurrent(context::UiCtxProjectFocused);
    } else if (CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB == scContext) {
        if (!matchWithCurrent(context::UiCtxProjectFocused)) {
            return false;
        }
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            return false;
        }
        auto noteInput = notation->interaction()->noteInput();
        return !noteInput->isNoteInputMode() || noteInput->state().staffGroup() != mu::engraving::StaffGroup::TAB;
    } else if (CTX_NOTATION_NOTE_INPUT_STAFF_TAB == scContext) {
        if (!matchWithCurrent(context::UiCtxProjectFocused)) {
            return false;
        }
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            return false;
        }
        auto noteInput = notation->interaction()->noteInput();
        return noteInput->isNoteInputMode() && noteInput->state().staffGroup() == mu::engraving::StaffGroup::TAB;
    } else if (CTX_NOTATION_TEXT_EDITING == scContext) {
        if (!matchWithCurrent(context::UiCtxProjectFocused)) {
            return false;
        }
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            return false;
        }
        return notation->interaction()->isTextEditingStarted();
    } else if (CTX_NOTATION_LIST_SELECTION == scContext) {
        if (!matchWithCurrent(context::UiCtxProjectFocused)) {
            return false;
        }
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            return false;
        }
        return !notation->interaction()->selection()->isRange();
    }

    IF_ASSERT_FAILED(CTX_ANY == scContext) {
        return true;
    }
    return true;
}
