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
#pragma once

#include <QString>

#include "async/notification.h"
#include "internal/inotationundostack.h"
#include "notationtypes.h"
#include "inotationpainting.h"
#include "inotationviewstate.h"
#include "inotationsolomutestate.h"
#include "inotationstyle.h"
#include "inotationelements.h"
#include "inotationinteraction.h"
#include "inotationaccessibility.h"
#include "inotationmidiinput.h"
#include "inotationparts.h"
#include "notationtypes.h"

namespace mu::notation {
class INotation;
using INotationPtr = std::shared_ptr<INotation>;
using INotationWeakPtr = std::weak_ptr<INotation>;
using INotationPtrList = std::vector<INotationPtr>;

class INotation
{
public:
    virtual ~INotation() = default;

    /// For MasterScores: the filename without extension
    /// For Scores: the excerpt name
    virtual QString name() const = 0;

    /// Filename without extension
    virtual QString projectName() const = 0;
    virtual QString projectNameAndPartName() const = 0;

    /// Title from score meta information; uses filename as fallback
    virtual QString workTitle() const = 0;
    virtual QString projectWorkTitle() const = 0;
    virtual QString projectWorkTitleAndPartName() const = 0;

    virtual bool isOpen() const = 0;
    virtual void setIsOpen(bool opened) = 0;
    virtual muse::async::Notification openChanged() const = 0;

    virtual bool hasVisibleParts() const = 0;

    virtual bool isMaster() const = 0;

    // draw
    virtual ViewMode viewMode() const = 0;
    virtual void setViewMode(const ViewMode& viewMode) = 0;
    virtual muse::async::Notification viewModeChanged() const = 0;

    virtual INotationPaintingPtr painting() const = 0;
    virtual INotationViewStatePtr viewState() const = 0;

    // solo-mute state
    virtual INotationSoloMuteStatePtr soloMuteState() const = 0;

    // input (mouse)
    virtual INotationInteractionPtr interaction() const = 0;

    // input (midi)
    virtual INotationMidiInputPtr midiInput() const = 0;

    // undo stack
    virtual INotationUndoStackPtr undoStack() const = 0;

    // styles
    virtual INotationStylePtr style() const = 0;

    // elements
    virtual INotationElementsPtr elements() const = 0;

    // accessibility
    virtual INotationAccessibilityPtr accessibility() const = 0;

    // parts
    virtual INotationPartsPtr parts() const = 0;

    // notify
    virtual muse::async::Notification notationChanged() const = 0;
};
}
