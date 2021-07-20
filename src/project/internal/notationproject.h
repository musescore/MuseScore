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
#ifndef MU_PROJECT_NOTATIONPROJECT_H
#define MU_PROJECT_NOTATIONPROJECT_H

#include "../inotationproject.h"

#include "modularity/ioc.h"
#include "inotationreadersregister.h"
#include "inotationwritersregister.h"
#include "system/ifilesystem.h"

#include "engraving/engravingproject.h"

#include "notation/internal/masternotation.h"
#include "projectaudiosettings.h"

namespace mu::engraving {
class MscReader;
class MscWriter;
}

namespace mu::project {
class NotationProject : public INotationProject
{
    INJECT(project, system::IFileSystem, fileSystem)
    INJECT(project, INotationReadersRegister, readers)
    INJECT(project, INotationWritersRegister, writers)

public:
    NotationProject();

    io::path path() const override;

    Ret load(const io::path& path, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret createNew(const ProjectCreateOptions& projectInfo) override;

    RetVal<bool> created() const override;
    ValNt<bool> needSave() const override;

    Ret save(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save) override;
    Ret writeToDevice(io::Device* device) override;

    notation::Meta metaInfo() const override;
    void setMetaInfo(const notation::Meta& meta) override;

    notation::IMasterNotationPtr masterNotation() const override;
    IProjectAudioSettingsPtr audioSettings() const override;

private:

    Ret doLoad(engraving::MscReader& reader, const io::path& stylePath, bool forceMode);
    Ret doImport(const io::path& path, const io::path& stylePath, bool forceMode);

    Ret saveScore(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save);
    Ret saveSelectionOnScore(const io::path& path = io::path());
    Ret exportProject(const io::path& path, const std::string& suffix);
    Ret doSave(bool generateBackup);
    Ret writeProject(engraving::MscWriter& msczWriter, bool onlySelection);

    mu::engraving::EngravingProjectPtr m_engravingProject = nullptr;
    notation::MasterNotationPtr m_masterNotation = nullptr;
    ProjectAudioSettingsPtr m_projectAudioSettings = nullptr;
};
}

#endif // MU_PROJECT_NOTATIONPROJECT_H
