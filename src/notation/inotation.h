//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_INOTATION_H
#define MU_NOTATION_INOTATION_H

#include "async/notification.h"
#include "internal/inotationundostack.h"
#include "notationtypes.h"
#include "inotationstyle.h"
#include "inotationplayback.h"
#include "inotationelements.h"
#include "inotationinteraction.h"
#include "inotationaccessibility.h"
#include "inotationmidiinput.h"
#include "inotationparts.h"
#include "notationtypes.h"

class QString;
class QRect;

namespace mu::notation {
class INotation;
using INotationPtr = std::shared_ptr<INotation>;

class INotation
{
public:
    virtual ~INotation() = default;

    virtual Meta metaInfo() const = 0;
    virtual void setMetaInfo(const Meta& meta) = 0;

    virtual INotationPtr clone() const = 0;

    virtual void setViewSize(const QSizeF& vs) = 0;
    virtual void setViewMode(const ViewMode& vm) = 0;
    virtual ViewMode viewMode() const = 0;
    virtual void paint(mu::draw::Painter* painter, const QRectF& frameRect) = 0;

    virtual ValCh<bool> opened() const = 0;
    virtual void setOpened(bool opened) = 0;

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
