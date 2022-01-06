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
}

void ProjectAutoSaver::onTrySave()
{
    INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        LOGD() << "[autosave] no project";
        return;
    }

    if (project->created().val) {
        LOGD() << "[autosave] project just created";
        return;
    }

    if (!project->needSave().val) {
        LOGD() << "[autosave] project does not need save";
        return;
    }

    Ret ret = project->save(mu::io::path(), SaveMode::Autosave);
    if (!ret) {
        LOGE() << "[autosave] failed to save project, err: " << ret.toString();
        return;
    }

    LOGD() << "[autosave] successfully saved project";
}
