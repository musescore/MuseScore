/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <gmock/gmock.h>

#include "project/inotationproject.h"
#include "project/types/projectmeta.h"

namespace mu::project {
class NotationProjectMock : public INotationProject
{
public:
    MOCK_METHOD(muse::io::path_t, path, (), (const, override));
    MOCK_METHOD(void, setPath, (const muse::io::path_t& path), (override));
    MOCK_METHOD(muse::async::Notification, pathChanged, (), (const, override));

    MOCK_METHOD(QString, displayName, (), (const, override));
    MOCK_METHOD(muse::async::Notification, displayNameChanged, (), (const, override));

    MOCK_METHOD(muse::Ret, load, (const muse::io::path_t& path, const OpenParams& params, const std::string& format), (override));
    MOCK_METHOD(muse::Ret, createNew, (const ProjectCreateOptions& projectInfo), (override));

    MOCK_METHOD(bool, isCloudProject, (), (const, override));
    MOCK_METHOD(const CloudProjectInfo&, cloudInfo, (), (const, override));
    MOCK_METHOD(void, setCloudInfo, (const CloudProjectInfo& info), (override));
    MOCK_METHOD(const CloudAudioInfo&, cloudAudioInfo, (), (const, override));
    MOCK_METHOD(void, setCloudAudioInfo, (const CloudAudioInfo& audioInfo), (override));

    MOCK_METHOD(bool, isNewlyCreated, (), (const, override));
    MOCK_METHOD(void, markAsNewlyCreated, (), (override));
    MOCK_METHOD(bool, isImported, (), (const, override));
    MOCK_METHOD(void, markAsUnsaved, (), (override));
    MOCK_METHOD(bool, isNeedSave, (), (const, override));
    MOCK_METHOD(muse::async::Notification, needSaveChanged, (), (const, override));

    MOCK_METHOD(muse::Ret, canSave, (), (const, override));
    MOCK_METHOD(bool, needAutoSave, (), (const, override));
    MOCK_METHOD(void, setNeedAutoSave, (bool val), (override));

    MOCK_METHOD(muse::Ret, save, (const muse::io::path_t& path, SaveMode saveMode, bool createBackup), (override));
    MOCK_METHOD(muse::Ret, savePage, (const muse::io::path_t& path, const size_t pageNum), (override));
    MOCK_METHOD((muse::async::Channel<muse::io::path_t, SaveMode>), saveComplited, (), (const, override));
    MOCK_METHOD(muse::Ret, writeToDevice, (QIODevice * device), (override));

    MOCK_METHOD(ProjectMeta, metaInfo, (), (const, override));
    MOCK_METHOD(void, setMetaInfo, (const ProjectMeta& meta, bool undoable), (override));

    MOCK_METHOD(notation::IMasterNotationPtr, masterNotation, (), (const, override));
    MOCK_METHOD(IProjectAudioSettingsPtr, audioSettings, (), (const, override));
};
}
