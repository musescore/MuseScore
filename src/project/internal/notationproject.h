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
#include "io/ifilesystem.h"
#include "../iprojectconfiguration.h"
#include "inotationreadersregister.h"
#include "inotationwritersregister.h"

#include "engraving/engravingproject.h"

#include "notation/inotationcreator.h"
#include "notation/inotationconfiguration.h"
#include "projectaudiosettings.h"
#include "iprojectmigrator.h"

namespace mu::engraving {
class MscReader;
class MscWriter;
}

namespace mu::project {
class NotationProject : public INotationProject, public async::Asyncable
{
    INJECT(io::IFileSystem, fileSystem)
    INJECT(IProjectConfiguration, configuration)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(notation::INotationCreator, notationCreator)
    INJECT(INotationReadersRegister, readers)
    INJECT(INotationWritersRegister, writers)
    INJECT(IProjectMigrator, migrator)

public:
    ~NotationProject() override;

    Ret load(const io::path_t& path, const io::path_t& stylePath = io::path_t(), bool forceMode = false,
             const std::string& format = "") override;
    Ret createNew(const ProjectCreateOptions& projectInfo) override;

    io::path_t path() const override;
    void setPath(const io::path_t& path) override;
    async::Notification pathChanged() const override;

    QString displayName() const override;
    async::Notification displayNameChanged() const override;

    bool isCloudProject() const override;
    const CloudProjectInfo& cloudInfo() const override;
    void setCloudInfo(const CloudProjectInfo& info) override;

    const CloudAudioInfo& cloudAudioInfo() const override;
    void setCloudAudioInfo(const CloudAudioInfo& audioInfo) override;

    bool isNewlyCreated() const override;
    void markAsNewlyCreated() override;

    bool isImported() const override;

    void markAsUnsaved() override;

    ValNt<bool> needSave() const override;
    Ret canSave() const override;

    bool needAutoSave() const override;
    void setNeedAutoSave(bool val) override;

    Ret save(const io::path_t& path = io::path_t(), SaveMode saveMode = SaveMode::Save) override;
    Ret writeToDevice(QIODevice* device) override;

    ProjectMeta metaInfo() const override;
    void setMetaInfo(const ProjectMeta& meta, bool undoable = false) override;

    notation::IMasterNotationPtr masterNotation() const override;
    IProjectAudioSettingsPtr audioSettings() const override;

private:
    void setupProject();

    Ret loadTemplate(const ProjectCreateOptions& projectOptions);

    Ret doLoad(const io::path_t& path, const io::path_t& stylePath, bool forceMode, const std::string& format);
    Ret doImport(const io::path_t& path, const io::path_t& stylePath, bool forceMode);

    Ret saveScore(const io::path_t& path, const std::string& fileSuffix, bool generateBackup = true, bool createThumbnail = true);
    Ret saveSelectionOnScore(const io::path_t& path = io::path_t());
    Ret exportProject(const io::path_t& path, const std::string& suffix);
    Ret doSave(const io::path_t& path, engraving::MscIoMode ioMode, bool generateBackup = true, bool createThumbnail = true);
    Ret makeCurrentFileAsBackup();
    Ret writeProject(engraving::MscWriter& msczWriter, bool onlySelection, bool createThumbnail = true);

    void listenIfNeedSaveChanges();
    void markAsSaved(const io::path_t& path);
    void setNeedSave(bool needSave);

    mu::engraving::EngravingProjectPtr m_engravingProject = nullptr;
    notation::IMasterNotationPtr m_masterNotation = nullptr;
    ProjectAudioSettingsPtr m_projectAudioSettings = nullptr;
    mutable CloudProjectInfo m_cloudInfo;
    mutable CloudAudioInfo m_cloudAudioInfo;

    io::path_t m_path;
    async::Notification m_pathChanged;
    async::Notification m_displayNameChanged;

    async::Notification m_needSaveNotification;

    bool m_isNewlyCreated = false; /// true if the file has never been saved yet
    bool m_isImported = false;
    bool m_needAutoSave = false;
    bool m_hasNonUndoStackChanges = false;
};
}

#endif // MU_PROJECT_NOTATIONPROJECT_H
