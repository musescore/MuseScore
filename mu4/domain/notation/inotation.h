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
#ifndef MU_DOMAIN_INOTATION_H
#define MU_DOMAIN_INOTATION_H

#include <QRect>
#include <string>

#include "modularity/imoduleexport.h"
#include "actions/action.h"
#include "async/notify.h"
#include "inotationinputstate.h"
#include "inotationselection.h"
#include "inotationinputcontroller.h"
#include "notationtypes.h"

class QPainter;
namespace mu {
namespace domain {
namespace notation {
class INotation : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::domain::notation::INotation)

public:
    ~INotation() = default;

    virtual bool load(const std::string& path) = 0;
    virtual void setViewSize(const QSizeF& vs) = 0;
    virtual void paint(QPainter* p, const QRect& r) = 0;

    // Edit
    virtual INotationInputState* inputState() const = 0;
    virtual void startNoteEntry() = 0;
    virtual void endNoteEntry() = 0;
    virtual void padNote(const Pad& pad) = 0;
    virtual void putNote(const QPointF& pos, bool replace, bool insert) = 0;

    // shadow note
    virtual void showShadowNote(const QPointF& p) = 0;
    virtual void hideShadowNote() = 0;
    virtual void paintShadowNote(QPainter* p) = 0;

    // Select
    virtual INotationSelection* selection() const = 0;
    virtual void select(Element* e, SelectType type, int staffIdx = 0) = 0;

    // Input (mouse)
    virtual INotationInputController* inputController() const = 0;

    // notify
    virtual async::Notify notationChanged() const = 0;
    virtual async::Notify inputStateChanged() const = 0;
    virtual async::Notify selectionChanged() const = 0;
};
}
}
}

#endif // MU_DOMAIN_INOTATION_H
