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
#ifndef MU_PROJECT_INOTATIONPROJECT_H
#define MU_PROJECT_INOTATIONPROJECT_H

#include <memory>

#include "global/io/path.h"
#include "global/types/ret.h"
#include "global/async/channel.h"

#include "iprojectaudiosettings.h"
#include "notation/imasternotation.h"
#include "types/projecttypes.h"

namespace mu::project {
class INotationProject
{
public:
    virtual ~INotationProject() = default;

    virtual muse::io::path_t path() const = 0;
    virtual void setPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Notification pathChanged() const = 0;

    virtual QString displayName() const = 0;
    virtual muse::async::Notification displayNameChanged() const = 0;

    virtual muse::Ret load(const muse::io::path_t& path,
                           const muse::io::path_t& stylePath = muse::io::path_t(), bool forceMode = false,
                           const std::string& format = "") = 0;
    virtual muse::Ret createNew(const ProjectCreateOptions& projectInfo) = 0;

    virtual bool isCloudProject() const = 0;
    virtual const CloudProjectInfo& cloudInfo() const = 0;
    virtual void setCloudInfo(const CloudProjectInfo& info) = 0;

    virtual const CloudAudioInfo& cloudAudioInfo() const = 0;
    virtual void setCloudAudioInfo(const CloudAudioInfo& audioInfo) = 0;

    virtual bool isNewlyCreated() const = 0;
    virtual void markAsNewlyCreated() = 0;

    virtual bool isImported() const = 0;

    virtual void markAsUnsaved() = 0;

    virtual muse::ValNt<bool> needSave() const = 0;
    virtual muse::Ret canSave() const = 0;

    virtual bool needAutoSave() const = 0;
    virtual void setNeedAutoSave(bool val) = 0;

    virtual muse::Ret save(const muse::io::path_t& path = muse::io::path_t(), SaveMode saveMode = SaveMode::Save,
                           bool createBackup = true) = 0;
    virtual muse::async::Channel<muse::io::path_t, SaveMode> saveComplited() const = 0;

    virtual muse::Ret writeToDevice(QIODevice* device) = 0;

    virtual ProjectMeta metaInfo() const = 0;
    virtual void setMetaInfo(const ProjectMeta& meta, bool undoable = false) = 0;

    virtual notation::IMasterNotationPtr masterNotation() const = 0;
    virtual IProjectAudioSettingsPtr audioSettings() const = 0;
};

using INotationProjectPtr = std::shared_ptr<INotationProject>;
}

#endif // MU_PROJECT_INOTATIONPROJECT_H
