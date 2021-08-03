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

constexpr int SAVE_INTERVAL_MS(1000 * 60 * 3);

void ProjectAutoSaver::init()
{
    QObject::connect(&m_timer, &QTimer::timeout, [this]() { onTrySave(); });
    m_timer.setSingleShot(false);
    m_timer.start(SAVE_INTERVAL_MS);
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

    Ret ret = project->save();
    if (!ret) {
        LOGE() << "[autosave] failed to save project, err: " << ret.toString();
        return;
    }

    LOGD() << "[autosave] successfully saved project";
}
