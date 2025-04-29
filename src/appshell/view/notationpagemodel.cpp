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
#include "notationpagemodel.h"

#include "internal/applicationuiactions.h"
#include "dockwindow/idockwindow.h"

#include "async/async.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace muse::actions;

NotationPageModel::NotationPageModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

bool NotationPageModel::isNavigatorVisible() const
{
    return configuration()->isNotationNavigatorVisible();
}

bool NotationPageModel::isBraillePanelVisible() const
{
    return brailleConfiguration()->braillePanelEnabled();
}

void NotationPageModel::init()
{
    TRACEFUNC;

    for (const ActionCode& actionCode : ApplicationUiActions::toggleDockActions().keys()) {
        DockName dockName = ApplicationUiActions::toggleDockActions()[actionCode];
        dispatcher()->reg(this, actionCode, [=]() { toggleDock(dockName); });
    }

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    brailleConfiguration()->braillePanelEnabledChanged().onNotify(this, [this]() {
        emit isBraillePanelVisibleChanged();
    });

    onNotationChanged();

    updateDrumsetPanelVisibility();
    updatePercussionPanelVisibility();

    notationConfiguration()->useNewPercussionPanelChanged().onNotify(this, [this]() {
        updateDrumsetPanelVisibility();
        updatePercussionPanelVisibility();
    });

    notationConfiguration()->percussionPanelAutoShowModeChanged().onNotify(this, [this]() {
        updatePercussionPanelVisibility();
    });
}

QString NotationPageModel::notationToolBarName() const
{
    return NOTATION_TOOLBAR_NAME;
}

QString NotationPageModel::playbackToolBarName() const
{
    return PLAYBACK_TOOLBAR_NAME;
}

QString NotationPageModel::undoRedoToolBarName() const
{
    return UNDO_REDO_TOOLBAR_NAME;
}

QString NotationPageModel::noteInputBarName() const
{
    return NOTE_INPUT_BAR_NAME;
}

QString NotationPageModel::palettesPanelName() const
{
    return PALETTES_PANEL_NAME;
}

QString NotationPageModel::layoutPanelName() const
{
    return LAYOUT_PANEL_NAME;
}

QString NotationPageModel::inspectorPanelName() const
{
    return INSPECTOR_PANEL_NAME;
}

QString NotationPageModel::selectionFiltersPanelName() const
{
    return SELECTION_FILTERS_PANEL_NAME;
}

QString NotationPageModel::undoHistoryPanelName() const
{
    return UNDO_HISTORY_PANEL_NAME;
}

QString NotationPageModel::mixerPanelName() const
{
    return MIXER_PANEL_NAME;
}

QString NotationPageModel::pianoKeyboardPanelName() const
{
    return PIANO_KEYBOARD_PANEL_NAME;
}

QString NotationPageModel::timelinePanelName() const
{
    return TIMELINE_PANEL_NAME;
}

QString NotationPageModel::drumsetPanelName() const
{
    return DRUMSET_PANEL_NAME;
}

QString NotationPageModel::percussionPanelName() const
{
    return PERCUSSION_PANEL_NAME;
}

QString NotationPageModel::statusBarName() const
{
    return NOTATION_STATUSBAR_NAME;
}

void NotationPageModel::onNotationChanged()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    INotationNoteInputPtr noteInput = notation->interaction()->noteInput();
    noteInput->stateChanged().onNotify(this, [this]() {
        updateDrumsetPanelVisibility();
        updatePercussionPanelVisibility();
    });

    INotationInteractionPtr notationInteraction = notation->interaction();
    notationInteraction->selectionChanged().onNotify(this, [this]() {
        updateDrumsetPanelVisibility();
        updatePercussionPanelVisibility();
    });
}

void NotationPageModel::toggleDock(const QString& name)
{
    if (name == NOTATION_NAVIGATOR_PANEL_NAME) {
        configuration()->setIsNotationNavigatorVisible(!isNavigatorVisible());
        emit isNavigatorVisibleChanged();
        return;
    }

    if (name == NOTATION_BRAILLE_PANEL_NAME) {
        brailleConfiguration()->setBraillePanelEnabled(!isBraillePanelVisible());
        emit isBraillePanelVisibleChanged();
        return;
    }

    dispatcher()->dispatch("dock-toggle", ActionData::make_arg1<QString>(name));
}

void NotationPageModel::updateDrumsetPanelVisibility()
{
    TRACEFUNC;

    const muse::dock::IDockWindow* window = dockWindowProvider()->window();
    if (!window) {
        return;
    }

    auto setDrumsetPanelOpen = [this, window](bool open) {
        if (open == window->isDockOpenAndCurrentInFrame(DRUMSET_PANEL_NAME)) {
            return;
        }

        //! NOTE: ensure we don't dispatch it multiple times in succession
        muse::async::Async::call(this, [=]() {
            dispatcher()->dispatch("dock-set-open", ActionData::make_arg2<QString, bool>(DRUMSET_PANEL_NAME, open));
        });
    };

    // This should never be open when the new percussion panel is in use...
    if (notationConfiguration()->useNewPercussionPanel()) {
        setDrumsetPanelOpen(false);
        return;
    }

    const INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        setDrumsetPanelOpen(false);
        return;
    }

    const INotationNoteInputPtr noteInput = notation->interaction()->noteInput();
    const bool shouldOpen = noteInput->isNoteInputMode() && noteInput->state().drumset() != nullptr;

    setDrumsetPanelOpen(shouldOpen);
}

void NotationPageModel::updatePercussionPanelVisibility()
{
    TRACEFUNC;

    //! NOTE: If the user is entering percussion notes with the piano keyboard, we can assume that they
    //! don't want the percussion panel to auto-show...
    const muse::dock::IDockWindow* window = dockWindowProvider()->window();
    if (!window || window->isDockOpenAndCurrentInFrame(PIANO_KEYBOARD_PANEL_NAME)) {
        return;
    }

    auto setPercussionPanelOpen = [this, window](bool open) {
        if (open == window->isDockOpenAndCurrentInFrame(PERCUSSION_PANEL_NAME)) {
            return;
        }

        //! NOTE: ensure we don't dispatch it multiple times in succession
        muse::async::Async::call(this, [=]() {
            dispatcher()->dispatch("dock-set-open", ActionData::make_arg2<QString, bool>(PERCUSSION_PANEL_NAME, open));
        });
    };

    // This should never be open when the old drumset panel is in use...
    if (!notationConfiguration()->useNewPercussionPanel()) {
        setPercussionPanelOpen(false);
        return;
    }

    const PercussionPanelAutoShowMode autoShowMode = notationConfiguration()->percussionPanelAutoShowMode();
    const INotationPtr notation = globalContext()->currentNotation();
    if (!notation || !notation->elements() || autoShowMode == PercussionPanelAutoShowMode::NEVER) {
        return;
    }

    const INotationNoteInputPtr noteInput = notation->interaction()->noteInput();
    const bool autoClose = notationConfiguration()->autoClosePercussionPanel();
    if (noteInput && !noteInput->isNoteInputMode() && autoShowMode == PercussionPanelAutoShowMode::UNPITCHED_STAFF_NOTE_INPUT) {
        if (autoClose) {
            setPercussionPanelOpen(false);
        }
        return;
    }

    const mu::engraving::Score* score = notation->elements()->msScore();
    const INotationSelectionPtr selection = notation->interaction()->selection();
    if (!score || !selection || selection->isNone()) {
        if (autoClose) {
            setPercussionPanelOpen(false);
        }
        return;
    }

    if (selection->isRange()) {
        const INotationSelectionRangePtr rangeSelection = selection->range();
        if (!rangeSelection) {
            if (autoClose) {
                setPercussionPanelOpen(false);
            }
            return;
        }
        for (const Part* p : rangeSelection->selectedParts()) {
            if (p->hasDrumStaff()) {
                continue;
            }
            if (autoClose) {
                setPercussionPanelOpen(false);
            }
            return;
        }
    } else {
        for (const EngravingItem* e : selection->elements()) {
            const Staff* staff = e->staff();
            if (staff && staff->isDrumStaff(e->tick())) {
                continue;
            }
            if (autoClose) {
                setPercussionPanelOpen(false);
            }
            return;
        }
    }

    setPercussionPanelOpen(true);
}
