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

namespace mu::project {
class NotationProjectMock : public INotationProject
{
public:
    MOCK_METHOD(muse::io::path_t, path, (), (const, override));
    MOCK_METHOD(void, setPath, (const muse::io::path_t&), (override));
    MOCK_METHOD(muse::async::Notification, pathChanged, (), (const, override));

    MOCK_METHOD(QString, displayName, (), (const, override));
    MOCK_METHOD(muse::async::Notification, displayNameChanged, (), (const, override));

    MOCK_METHOD(muse::Ret, load, (const muse::io::path_t&, const OpenParams&, const std::string&), (override));
    MOCK_METHOD(muse::Ret, createNew, (const ProjectCreateOptions&), (override));

    MOCK_METHOD(bool, isCloudProject, (), (const, override));
    MOCK_METHOD(const CloudProjectInfo&, cloudInfo, (), (const, override));
    MOCK_METHOD(void, setCloudInfo, (const CloudProjectInfo&), (override));

    MOCK_METHOD(const CloudAudioInfo&, cloudAudioInfo, (), (const, override));
    MOCK_METHOD(void, setCloudAudioInfo, (const CloudAudioInfo&), (override));

    MOCK_METHOD(bool, isNewlyCreated, (), (const, override));
    MOCK_METHOD(void, markAsNewlyCreated, (), (override));

    MOCK_METHOD(bool, isImported, (), (const, override));

    MOCK_METHOD(void, markAsUnsaved, (), (override));

    MOCK_METHOD(muse::ValNt<bool>, needSave, (), (const, override));
    MOCK_METHOD(muse::Ret, canSave, (), (const, override));

    MOCK_METHOD(bool, needAutoSave, (), (const, override));
    MOCK_METHOD(void, setNeedAutoSave, (bool), (override));

    MOCK_METHOD(muse::Ret, save, (const muse::io::path_t&, SaveMode, bool), (override));
    MOCK_METHOD(muse::Ret, savePage, (const muse::io::path_t&, const size_t), (override));
    MOCK_METHOD((muse::async::Channel<muse::io::path_t, SaveMode>), saveComplited, (), (const, override));

    MOCK_METHOD(muse::Ret, writeToDevice, (QIODevice*), (override));

    MOCK_METHOD(ProjectMeta, metaInfo, (), (const, override));
    MOCK_METHOD(void, setMetaInfo, (const ProjectMeta&, bool), (override));

    MOCK_METHOD(notation::IMasterNotationPtr, masterNotation, (), (const, override));
    MOCK_METHOD(IProjectAudioSettingsPtr, audioSettings, (), (const, override));
};
}
