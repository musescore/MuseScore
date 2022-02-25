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
#include "path.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include "stringutils.h"

using namespace mu::io;

path::path(const QByteArray& s)
    : m_path(s)
{
}

path::path(const QString& s)
    : m_path(s.toUtf8())
{
}

path::path(const std::string& s)
    : m_path(s.c_str())
{
}

path::path(const char* s)
    : m_path(s)
{
}

bool path::empty() const
{
    return m_path.isEmpty();
}

path path::appendingComponent(const path& other) const
{
    if (m_path.endsWith('/') || other.m_path.startsWith('/')) {
        return m_path + other.m_path;
    }

    return m_path + '/' + other.m_path;
}

path path::appendingSuffix(const path& suffix) const
{
    if (suffix.m_path.startsWith('.')) {
        return m_path + suffix.m_path;
    }

    return m_path + '.' + suffix.m_path;
}

QString path::toQString() const
{
    return QString(m_path);
}

std::string path::toStdString() const
{
    return std::string(m_path.data());
}

std::wstring path::toStdWString() const
{
    return QString(m_path).toStdWString();
}

const char* path::c_str() const
{
    return m_path.data();
}

paths path::pathsFromString(const std::string& str, const std::string& delim)
{
    std::vector<std::string> strs;
    strings::split(str, strs, delim);

    paths ps;
    ps.reserve(strs.size());
    for (const std::string& s : strs) {
        ps.push_back(path(s));
    }
    return ps;
}

std::string mu::io::suffix(const mu::io::path& path)
{
    QFileInfo fi(path.toQString());
    return fi.suffix().toLower().toStdString();
}

mu::io::path mu::io::filename(const mu::io::path& path, bool includingExtension)
{
    QFileInfo fi(path.toQString());
    return includingExtension ? fi.fileName() : fi.completeBaseName();
}

mu::io::path mu::io::basename(const mu::io::path& path)
{
    QFileInfo fi(path.toQString());
    return fi.baseName();
}

mu::io::path mu::io::absolutePath(const path& path)
{
    return QFileInfo(path.toQString()).absolutePath();
}

mu::io::path mu::io::dirname(const mu::io::path& path)
{
    return QFileInfo(path.toQString()).dir().dirName();
}

mu::io::path mu::io::dirpath(const mu::io::path& path)
{
    return QFileInfo(path.toQString()).dir().path();
}

mu::io::path mu::io::absoluteDirpath(const mu::io::path& path)
{
    return QFileInfo(path.toQString()).dir().absolutePath();
}

bool mu::io::isAbsolute(const path& path)
{
    return QFileInfo(path.toQString()).isAbsolute();
}

bool mu::io::isAllowedFileName(const path& fn_)
{
    QString fn = basename(fn_).toQString();

    // Windows filenames are not case sensitive.
    fn = fn.toUpper();

    static const QString illegal="<>:\"|?*";

    for (const QChar& c : fn) {
        // Check for control characters
        if (c.toLatin1() > 0 && c.toLatin1() < 32) {
            return false;
        }

        // Check for illegal characters
        if (illegal.contains(c)) {
            return false;
        }
    }

    // Check for device names in filenames
    static const QStringList devices = {
        "CON", "PRN",  "AUX",  "NUL",
        "COM0", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT0", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    foreach (const QString& s, devices) {
        if (fn == s) {
            return false;
        }
    }

    // Check for trailing periods or spaces
    if (fn.right(1) == "." || fn.right(1) == " ") {
        return false;
    }

    // Check for pathnames that are too long
    if (fn.length() > 96) {
        return false;
    }

    // Since we are checking for a filename, it mustn't be a directory
    if (fn.right(1) == "\\") {
        return false;
    }

    return true;
}

mu::io::path mu::io::escapeFileName(const mu::io::path& fn_)
{
    //
    // special characters in filenames are a constant source
    // of trouble, this replaces some of them common in german:
    //
    QString fn = fn_.toQString();
    fn = fn.simplified();
    fn = fn.replace(QChar(' '),  "_");
    fn = fn.replace(QChar('\n'), "_");
    fn = fn.replace(QChar(0xe4), "ae"); // &auml;
    fn = fn.replace(QChar(0xf6), "oe"); // &ouml;
    fn = fn.replace(QChar(0xfc), "ue"); // &uuml;
    fn = fn.replace(QChar(0xdf), "ss"); // &szlig;
    fn = fn.replace(QChar(0xc4), "Ae"); // &Auml;
    fn = fn.replace(QChar(0xd6), "Oe"); // &Ouml;
    fn = fn.replace(QChar(0xdc), "Ue"); // &Uuml;
    fn = fn.replace(QChar(0x266d), "b"); // musical flat sign, happen in instrument names, so can happen in part (file) names
    fn = fn.replace(QChar(0x266f), "#"); // musical sharp sign, can happen in titles, so can happen in score (file) names
    fn = fn.replace(QRegularExpression("[" + QRegularExpression::escape("\\/:*?\"<>|") + "]"), "_"); //FAT/NTFS special chars
    return fn;
}

mu::io::paths mu::io::pathsFromStrings(const QStringList& list)
{
    paths result;

    for (const QString& path : list) {
        result.push_back(path);
    }

    return result;
}
