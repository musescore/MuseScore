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
};

using WorkspacePtr = std::shared_ptr<Workspace>;
}

#endif // MU_WORKSPACE_WORKSPACE_H
