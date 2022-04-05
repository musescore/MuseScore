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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "iprojectconfiguration.h"
#include "inotationreadersregister.h"
#include "inotationwritersregister.h"
#include "iprojectautosaver.h"

#include "engraving/engravingproject.h"
#include "engraving/infrastructure/io/ifileinfoprovider.h"

#include "notation/internal/masternotation.h"
#include "projectaudiosettings.h"
#include "projectviewsettings.h"
#include "iprojectmigrator.h"

namespace mu::engraving {
class MscReader;
class MscWriter;
}

namespace mu::project {
class NotationProject : public INotationProject, public async::Asyncable
{
    INJECT(project, system::IFileSystem, fileSystem)
    INJECT(project, IProjectConfiguration, configuration)
    INJECT(project, INotationReadersRegister, readers)
    INJECT(project, INotationWritersRegister, writers)
    INJECT(project, IProjectMigrator, migrator)
    INJECT(project, IProjectAutoSaver, projectAutoSaver)

public:
    NotationProject();
    ~NotationProject() override;

    Ret load(const io::path& path, const io::path& stylePath = io::path(), bool forceMode = false) override;
    Ret createNew(const ProjectCreateOptions& projectInfo) override;

    io::path path() const override;
    async::Notification pathChanged() const override;

    QString displayName() const override;

    bool isCloudProject() const override;

    bool isNewlyCreated() const override;
    ValNt<bool> needSave() const override;

    Ret save(const io::path& path = io::path(), SaveMode saveMode = SaveMode::Save) override;
    Ret writeToDevice(io::Device* device) override;

    ProjectMeta metaInfo() const override;
    void setMetaInfo(const ProjectMeta& meta) override;

    notation::IMasterNotationPtr masterNotation() const override;
    IProjectAudioSettingsPtr audioSettings() const override;
    IProjectViewSettingsPtr viewSettings() const override;

private:
    Ret loadTemplate(const ProjectCreateOptions& projectOptions);

    Ret doLoad(engraving::MscReader& reader, const io::path& stylePath, bool forceMode);
    Ret doImport(const io::path& path, const io::path& stylePath, bool forceMode);

    void setPath(const io::path& path);

    Ret saveScore(const io::path& path, const std::string& fileSuffix);
    Ret saveSelectionOnScore(const io::path& path = io::path());
    Ret exportProject(const io::path& path, const std::string& suffix);
    Ret doSave(const io::path& path, bool generateBackup, engraving::MscIoMode ioMode);
    Ret makeCurrentFileAsBackup();
    Ret writeProject(engraving::MscWriter& msczWriter, bool onlySelection);

    mu::engraving::EngravingProjectPtr m_engravingProject = nullptr;
    notation::MasterNotationPtr m_masterNotation = nullptr;
    ProjectAudioSettingsPtr m_projectAudioSettings = nullptr;
    ProjectViewSettingsPtr m_viewSettings = nullptr;

    io::path m_path;
    async::Notification m_pathChanged;

    async::Notification m_needSaveNotification;
};
}

#endif // MU_PROJECT_NOTATIONPROJECT_H
