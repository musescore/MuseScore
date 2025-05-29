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
#include "mainwindowtitleprovider.h"
#include "translation.h"

using namespace mu::appshell;
using namespace mu::notation;

MainWindowTitleProvider::MainWindowTitleProvider(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void MainWindowTitleProvider::load()
{
    update();

    context()->currentProjectChanged().onNotify(this, [this]() {
        if (auto currentProject = context()->currentProject()) {
            currentProject->displayNameChanged().onNotify(this, [this]() {
                update();
            });

            currentProject->needSave().notification.onNotify(this, [this]() {
                update();
            });
        }
    });

    context()->currentNotationChanged().onNotify(this, [this]() {
        update();
    });
}

QString MainWindowTitleProvider::title() const
{
    return m_title;
}

QString MainWindowTitleProvider::filePath() const
{
    return m_filePath;
}

bool MainWindowTitleProvider::fileModified() const
{
    return m_fileModified;
}

void MainWindowTitleProvider::setTitle(const QString& title)
{
    if (title == m_title) {
        return;
    }

    m_title = title;
    emit titleChanged(title);
}

void MainWindowTitleProvider::setFilePath(const QString& filePath)
{
    if (filePath == m_filePath) {
        return;
    }

    m_filePath = filePath;
    emit filePathChanged(filePath);
}

void MainWindowTitleProvider::setFileModified(bool fileModified)
{
    if (fileModified == m_fileModified) {
        return;
    }

    m_fileModified = fileModified;
    emit fileModifiedChanged(fileModified);
}

void MainWindowTitleProvider::update()
{
    project::INotationProjectPtr project = context()->currentProject();

    if (!project) {
        setTitle(muse::qtrc("appshell", "MuseScore Studio"));
        setFilePath("");
        setFileModified(false);
        return;
    }

    INotationPtr notation = context()->currentNotation();
    setTitle(notation->projectNameAndPartName());

    setFilePath((project->isNewlyCreated() || project->isCloudProject())
                ? "" : project->path().toQString());
    setFileModified(project->needSave().val);
}
