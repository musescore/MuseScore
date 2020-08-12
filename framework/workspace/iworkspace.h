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
#ifndef MU_WORKSPACE_IWORKSPACE_H
#define MU_WORKSPACE_IWORKSPACE_H

#include <memory>
#include <QString>
#include <QByteArray>

#include "workspace/workspacetypes.h"

namespace mu {
namespace workspace {
class IWorkspace
{
public:
    virtual ~IWorkspace() = default;

    virtual std::string name() const = 0;
    virtual std::string title() const = 0;

    virtual std::shared_ptr<AbstractData> data(const std::string& tag,const std::string& name = std::string()) const = 0;
    virtual void addData(std::shared_ptr<AbstractData> data) = 0;

    //! NOTE Only methods associations with framework.
    //! Other methods (for other data) must be in the appropriate modules.
    virtual Val settingValue(const std::string& name) const = 0;
    virtual std::vector<std::string> toolbarActions(const std::string& toolbarName) const = 0;
};
}
}

#endif // MU_WORKSPACE_IWORKSPACE_H
