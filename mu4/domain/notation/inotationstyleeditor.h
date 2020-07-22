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
#ifndef MU_DOMAIN_INOTATIONSTYLEEDITOR_H
#define MU_DOMAIN_INOTATIONSTYLEEDITOR_H

#include <QString>
#include <QList>

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "async/notification.h"

#include "notationtypes.h"

namespace mu {
namespace domain {
namespace notation {
class INotationStyleEditor : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationStyleEditor)

public:
    ~INotationStyleEditor() = default;

    virtual Style style() const = 0;
    virtual void changeStyle(ChangeStyleVal* newStyleValue) = 0;

    virtual void update() = 0;

    virtual bool isMaster() const = 0;
    virtual QList<QMap<QString, QString>> metaTags() const = 0;
    virtual QString textStyleUserName(Tid tid) = 0;
    virtual void setConcertPitch(bool status) = 0;

    virtual void startEdit() = 0;
    virtual void apply() = 0;
    virtual void applyAllParts() = 0;
    virtual void cancel() = 0;

    virtual async::Notification styleChanged() const = 0;
};
}
}
}

#endif // MU_DOMAIN_INOTATIONSTYLEEDITOR_H
