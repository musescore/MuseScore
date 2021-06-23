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
#ifndef MU_WORKSPACE_WORKSPACE_H
#define MU_WORKSPACE_WORKSPACE_H

#include <map>

#include "../iworkspace.h"
#include "modularity/ioc.h"
#include "../iworkspacedatastreamregister.h"
#include "io/path.h"
#include "ret.h"

namespace mu::workspace {
class Workspace : public IWorkspace
{
    INJECT(workspace, IWorkspaceDataStreamRegister, streamRegister)

public:
    Workspace(const io::path& filePath);

    std::string name() const override;
    std::string title() const override;

    RetVal<QByteArray> readRawData(const std::string& name) const override;
    Ret writeRawData(const std::string& name, const QByteArray& data) override;

    RetVal<Data> readData(const std::string& name) const override;
    Ret writeData(const std::string& name, const Data& data) override;

    // =======================================

    WorkspaceTagList tags() const override;
    void setTags(const WorkspaceTagList& tags) override;

    AbstractDataPtr data(WorkspaceTag tag, const std::string& name = std::string()) const override;
    AbstractDataPtrList dataList(WorkspaceTag tag) const override;
    void addData(AbstractDataPtr data) override;
    async::Channel<AbstractDataPtr> dataChanged() const override;

    Val settingValue(const std::string& key) const override;
    std::vector<std::string> toolbarActions(const std::string& toolbarName) const override;

    bool isInited() const;
    io::path filePath() const;
    Ret read();
    Ret write();

private:
    Ret readWorkspace(const QByteArray& data);
    void clear();

    std::string tagsNames() const;
    std::vector<WorkspaceTag> parseTags(const std::string& tagsStr) const;

    io::path m_filePath;
    bool m_isInited = false;
    bool m_hasUnsavedChanges = false;
    std::string m_title;
    std::string m_source;

    struct DataKey
    {
        WorkspaceTag tag;
        std::string name;

        inline bool operator ==(const DataKey& k) const { return k.tag == tag && k.name == name; }
        inline bool operator <(const DataKey& k) const
        {
            if (k.tag != tag) {
                return tag < k.tag;
            }
            return name < k.name;
        }
    };

    std::map<DataKey, AbstractDataPtr> m_data;
    async::Channel<AbstractDataPtr> m_dataChanged;

    WorkspaceTagList m_tags;
};

using WorkspacePtr = std::shared_ptr<Workspace>;
}

#endif // MU_WORKSPACE_WORKSPACE_H
