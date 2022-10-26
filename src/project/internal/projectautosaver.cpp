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
#include "projectautosaver.h"

#include "engraving/infrastructure/mscio.h"

#include "log.h"

using namespace mu::project;

void ProjectAutoSaver::init()
{
    QObject::connect(&m_timer, &QTimer::timeout, [this]() { onTrySave(); });
    m_timer.setSingleShot(false);
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setInterval(configuration()->autoSaveIntervalMinutes() * 60000);

    if (configuration()->isAutoSaveEnabled()) {
        m_timer.start();
    }

    configuration()->autoSaveEnabledChanged().onReceive(this, [this](bool enabled) {
        if (enabled != m_timer.isActive()) {
            if (enabled) {
                m_timer.start();
            } else {
                m_timer.stop();
            }
        }
    });

    configuration()->autoSaveIntervalChanged().onReceive(this, [this](int minutes) {
        m_timer.setInterval(minutes * 60000);
    });

    update();

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        if (auto project = currentProject()) {
            if (project->isNewlyCreated() && !project->isImported()) {
                Ret ret = project->save(configuration()->newProjectTemporaryPath(), SaveMode::AutoSave);
                if (!ret) {
                    LOGE() << "[autosave] failed to save project, err: " << ret.toString();
                    return;
                }
            }

            project->pathChanged().onNotify(this, [this]() {
                update();
            });

            project->needSave().notification.onNotify(this, [this]() {
                update();
            });
        }

        update();
    });
}

bool ProjectAutoSaver::projectHasUnsavedChanges(const io::path_t& projectPath) const
{
    io::path_t autoSavePath = projectAutoSavePath(projectPath);
    return fileSystem()->exists(autoSavePath);
}

void ProjectAutoSaver::removeProjectUnsavedChanges(const io::path_t& projectPath)
{
    io::path_t path = projectPath;
    if (!isAutosaveOfNewlyCreatedProject(projectPath)) {
        path = projectAutoSavePath(projectPath);
    }

    fileSystem()->remove(path);
}

bool ProjectAutoSaver::isAutosaveOfNewlyCreatedProject(const io::path_t& projectPath) const
{
    return projectPath == configuration()->newProjectTemporaryPath();
}

mu::io::path_t ProjectAutoSaver::projectOriginalPath(const mu::io::path_t& projectAutoSavePath) const
{
    IF_ASSERT_FAILED(io::suffix(projectAutoSavePath) == AUTOSAVE_SUFFIX) {
        return engraving::mainFilePath(projectAutoSavePath);
    }

    io::path_t withoutAutosaveSuffix = io::filename(projectAutoSavePath, false);

    return engraving::mainFilePath(io::absoluteDirpath(projectAutoSavePath).appendingComponent(withoutAutosaveSuffix));
}

mu::io::path_t ProjectAutoSaver::projectAutoSavePath(const io::path_t& projectPath) const
{
    return engraving::containerPath(projectPath).appendingSuffix(AUTOSAVE_SUFFIX);
}

INotationProjectPtr ProjectAutoSaver::currentProject() const
{
    return globalContext()->currentProject();
}

void ProjectAutoSaver::update()
{
    io::path_t newProjectPath;

    auto project = currentProject();
    if (project && project->needSave().val) {
        newProjectPath = projectPath(project);
    }

    if (!m_lastProjectPathNeedingAutosave.empty()
        && m_lastProjectPathNeedingAutosave != newProjectPath) {
        removeProjectUnsavedChanges(m_lastProjectPathNeedingAutosave);
    }

    m_lastProjectPathNeedingAutosave = newProjectPath;
}

void ProjectAutoSaver::onTrySave()
{
    INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        LOGD() << "[autosave] no project";
        return;
    }

    if (!project->needSave().val) {
        LOGD() << "[autosave] project does not need save";
        return;
    }

    if (!project->canSave()) {
        LOGD() << "[autosave] project could not be saved";
        return;
    }

    io::path_t projectPath = this->projectPath(project);
    io::path_t savePath = project->isNewlyCreated() ? projectPath : projectAutoSavePath(projectPath);

    Ret ret = project->save(savePath, SaveMode::AutoSave);
    if (!ret) {
        LOGE() << "[autosave] failed to save project, err: " << ret.toString();
        return;
    }

    LOGD() << "[autosave] successfully saved project";
}

mu::io::path_t ProjectAutoSaver::projectPath(INotationProjectPtr project) const
{
    return project->isNewlyCreated() ? configuration()->newProjectTemporaryPath() : project->path();
}
