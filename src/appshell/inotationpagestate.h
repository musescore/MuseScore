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
#ifndef MU_APPSHELL_INOTATIONPAGESTATE_H
#define MU_APPSHELL_INOTATIONPAGESTATE_H

#include "modularity/imoduleexport.h"
#include "appshelltypes.h"
#include "async/channel.h"

namespace mu::appshell {
class INotationPageState : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationPageState)

public:
    virtual ~INotationPageState() = default;

    virtual bool isPanelVisible(PanelType type) const = 0;
    virtual void setIsPanelsVisible(const std::map<PanelType, bool>& panelsVisible) = 0;
    virtual async::Channel<PanelTypeList> panelsVisibleChanged() const = 0;
};
}

#endif // MU_APPSHELL_NOTATIONPAGESTATE_H
