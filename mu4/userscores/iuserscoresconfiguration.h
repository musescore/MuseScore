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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_USERSCORES_IUSERSCORESCONFIGURATION_H
#define MU_USERSCORES_IUSERSCORESCONFIGURATION_H

#include <QStringList>
#include <QColor>

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"

namespace mu {
namespace userscores {
class IUserScoresConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUserScoresConfiguration)

public:
    virtual ~IUserScoresConfiguration() = default;

    virtual ValCh<QStringList> recentScoreList() const = 0;
    virtual void setRecentScoreList(const QStringList& recentScoreList) = 0;

    virtual io::paths templatesDirPaths() const = 0;

    virtual QColor templatePreviewBackgroundColor() const = 0;
    virtual async::Channel<QColor> templatePreviewBackgroundColorChanged() const = 0;
};
}
}

#endif // MU_USERSCORES_IUSERSCORESCONFIGURATION_H
