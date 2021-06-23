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

#include "../iworkspace.h"
#include "io/path.h"

namespace mu::workspace {
class Workspace : public IWorkspace
{
public:
    Workspace(const io::path& filePath);

    std::string name() const override;
    std::string title() const override;

    bool isManaged(const Option& key) const override;
    void setIsManaged(const Option& key, bool val) const override;

    RetVal<QByteArray> readRawData(const std::string& name) const override;
    Ret writeRawData(const std::string& name, const QByteArray& data) override;

    RetVal<Data> readData(const std::string& name) const override;
    Ret writeData(const std::string& name, const Data& data) override;

    // =======================================

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
};

using WorkspacePtr = std::shared_ptr<Workspace>;
}

#endif // MU_WORKSPACE_WORKSPACE_H
