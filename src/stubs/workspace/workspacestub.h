/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_WORKSPACE_WORKSPACESTUB_H
#define MU_WORKSPACE_WORKSPACESTUB_H

#include "workspace/iworkspace.h"

namespace mu::workspace {
class WorkspaceStub : public IWorkspace
{
public:
    std::string name() const override;
    std::string title() const override;

    AbstractDataPtr data(WorkspaceTag tag, const std::string& name = std::string()) const override;
    AbstractDataPtrList dataList(WorkspaceTag tag) const override;
    void addData(AbstractDataPtr data) override;
    async::Channel<AbstractDataPtr> dataChanged() const override;

    Val settingValue(const std::string& key) const override;
    std::vector<std::string> toolbarActions(const std::string& toolbarName) const override;
};
}

#endif // MU_WORKSPACE_WORKSPACESTUB_H
