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
#ifndef MU_NOTATION_INOTATION_H
#define MU_NOTATION_INOTATION_H

#include <QString>

#include "async/notification.h"
#include "internal/inotationundostack.h"
#include "notationtypes.h"
#include "inotationpainting.h"
#include "inotationstyle.h"
#include "inotationplayback.h"
#include "inotationelements.h"
#include "inotationinteraction.h"
#include "inotationaccessibility.h"
#include "inotationmidiinput.h"
#include "inotationparts.h"
#include "notationtypes.h"

namespace mu::notation {
class INotation;
using INotationPtr = std::shared_ptr<INotation>;
using INotationPtrList = std::vector<INotationPtr>;

class INotation
{
public:
    virtual ~INotation() = default;

    virtual QString title() const = 0;          //! NOTE Title of score (master) or title of part (excerpt)
    virtual QString completedTitle() const = 0; //! NOTE Title of score plus title of part (if is part)
    virtual QString scoreTitle() const = 0;     //! NOTE Title of score (master)

    virtual ValCh<bool> opened() const = 0;
    virtual void setOpened(bool opened) = 0;

    // draw
    virtual void setViewMode(const ViewMode& viewMode) = 0;
    virtual ViewMode viewMode() const = 0;
    virtual INotationPaintingPtr painting() const = 0;

    // input (mouse)
    virtual INotationInteractionPtr interaction() const = 0;

    // input (midi)
    virtual INotationMidiInputPtr midiInput() const = 0;

    // undo stack
    virtual INotationUndoStackPtr undoStack() const = 0;

    // styles
    virtual INotationStylePtr style() const = 0;

    // playback (midi)
    virtual INotationPlaybackPtr playback() const = 0;

    // elements
    virtual INotationElementsPtr elements() const = 0;

    // accessibility
    virtual INotationAccessibilityPtr accessibility() const = 0;

    // parts
    virtual INotationPartsPtr parts() const = 0;

    // notify
    virtual async::Notification notationChanged() const = 0;
};
}

#endif // MU_NOTATION_INOTATION_H
