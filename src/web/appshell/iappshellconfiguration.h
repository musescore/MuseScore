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
#ifndef MU_APPSHELL_IAPPSHELLCONFIGURATION_H
#define MU_APPSHELL_IAPPSHELLCONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "async/notification.h"

namespace mu::appshell {
class IAppShellConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAppshellConfiguration)

public:
    virtual ~IAppShellConfiguration() = default;

    virtual std::string museScoreVersion() const = 0;
    virtual std::string museScoreRevision() const = 0;

    virtual bool isNotationNavigatorVisible() const = 0;
    virtual void setIsNotationNavigatorVisible(bool visible) const = 0;
    virtual muse::async::Notification isNotationNavigatorVisibleChanged() const = 0;
};
}

#endif // MU_APPSHELL_IAPPSHELLCONFIGURATION_H
