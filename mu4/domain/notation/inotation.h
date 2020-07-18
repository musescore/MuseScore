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
#include "ret.h"
#include "io/path.h"
#include "async/notification.h"
#include "inotationinteraction.h"
#include "notationtypes.h"
#include "inotationreader.h"
#include "inotationplayback.h"

class QPainter;
namespace mu {
namespace domain {
namespace notation {
class INotation : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotation)

public:
    ~INotation() = default;

    virtual Ret load(const io::path& path) = 0;
    virtual Ret load(const io::path& path, const std::shared_ptr<INotationReader>& reader) = 0;
    virtual io::path path() const = 0;

    virtual Ret createNew(const ScoreCreateOptions& scoreInfo) = 0;

    virtual void setViewSize(const QSizeF& vs) = 0;
    virtual void paint(QPainter* p, const QRect& r) = 0;

    // input (mouse)
    virtual INotationInteraction* interaction() const = 0;

    // playback (midi)
    virtual INotationPlayback* playback() const = 0;

    // notify
    virtual async::Notification notationChanged() const = 0;
};
}
}
}

#endif // MU_DOMAIN_INOTATION_H
