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
#include "filedialog.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions::apiv1;

FileDialog::FileDialog(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this)) {}

QString FileDialog::doOpen(const QString& title, const QString& folder)
{
    io::path_t path;
    if (m_type == Load) {
        path = interactive()->selectOpeningFile(title, folder, {});
        LOGD() << "selectOpeningFile: " << path;
    } else {
        path = interactive()->selectSavingFile(title, folder, {});
        LOGD() << "selectSavingFile: " << path;
    }

    setVisible(false);

    return path.toQString();
}

QString FileDialog::title() const
{
    return m_title;
}

void FileDialog::setTitle(const QString& newTitle)
{
    if (m_title == newTitle) {
        return;
    }
    m_title = newTitle;
    emit titleChanged();
}

bool FileDialog::visible() const
{
    return m_visible;
}

void FileDialog::setVisible(bool newVisible)
{
    if (m_visible == newVisible) {
        return;
    }
    m_visible = newVisible;
    emit visibleChanged();

    if (m_visible) {
        m_filePath = doOpen(m_title, m_folder);
        if (!m_filePath.isEmpty()) {
            emit accepted();
        } else {
            emit rejected();
        }
    }
}

QString FileDialog::folder() const
{
    return m_folder;
}

void FileDialog::setFolder(const QString& newFolder)
{
    if (m_folder == newFolder) {
        return;
    }
    m_folder = newFolder;
    emit folderChanged();
}

QString FileDialog::filePath() const
{
    return m_filePath;
}

void FileDialog::setFilePath(const QString& newFilePath)
{
    if (m_filePath == newFilePath) {
        return;
    }
    m_filePath = newFilePath;
    emit filePathChanged();
}

FileDialog::Type FileDialog::type() const
{
    return m_type;
}

void FileDialog::setType(Type newType)
{
    if (m_type == newType) {
        return;
    }
    m_type = newType;
    emit typeChanged();
}
