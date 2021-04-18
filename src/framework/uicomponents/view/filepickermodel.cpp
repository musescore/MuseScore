//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "filepickermodel.h"

using namespace mu::uicomponents;

FilePickerModel::FilePickerModel(QObject* parent)
    : QObject(parent)
{
}

QString FilePickerModel::title() const
{
    return m_title;
}

QString FilePickerModel::dir() const
{
    return m_dir;
}

QString FilePickerModel::filter() const
{
    return m_filter;
}

QString FilePickerModel::selectFile()
{
    io::path file = interactive()->selectOpeningFile(m_title, m_dir, m_filter);
    return file.toQString();
}

QString FilePickerModel::selectDirectory()
{
    io::path directory = interactive()->selectDirectory(m_title, m_dir);
    return directory.toQString();
}

void FilePickerModel::setTitle(const QString& title)
{
    if (title == m_title) {
        return;
    }

    m_title = title;
    emit titleChanged(title);
}

void FilePickerModel::setDir(const QString& dir)
{
    if (dir == m_dir) {
        return;
    }

    m_dir = dir;
    emit dirChanged(dir);
}

void FilePickerModel::setFilter(const QString& filter)
{
    if (filter == m_filter) {
        return;
    }

    m_filter = filter;
    emit filterChanged(filter);
}
