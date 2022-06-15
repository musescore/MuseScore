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
#include "fileinfo.h"

using namespace mu;
using namespace mu::io;

path_t::path_t(const String& s)
    : m_path(s.toStdString())
{
}

path_t::path_t(const QString& s)
    : m_path(s.toStdString())
{
}

path_t::path_t(const std::string& s)
    : m_path(s)
{
}

path_t::path_t(const char* s)
    : m_path(s ? s : "")
{
}

bool path_t::empty() const
{
    return m_path.empty();
}

size_t path_t::size() const
{
    return m_path.size();
}

bool path_t::withSuffix(const char* str) const
{
    return io::FileInfo::suffix(m_path).toLower() == str;
}

path_t path_t::appendingComponent(const path_t& other) const
{
    if ((!m_path.empty() && m_path.back() == '/')
        || (!other.m_path.empty() && other.m_path.front() == '/')) {
        return m_path + other.m_path;
    }

    return m_path + '/' + other.m_path;
}

path_t path_t::appendingSuffix(const path_t& suffix) const
{
    if (!suffix.m_path.empty() && suffix.m_path.front() == '.') {
        return m_path + suffix.m_path;
    }

    return m_path + '.' + suffix.m_path;
}

String path_t::toString() const
{
    return String::fromStdString(m_path);
}

QString path_t::toQString() const
{
    return QString::fromStdString(m_path);
}

std::string path_t::toStdString() const
{
    return m_path;
}

std::wstring path_t::toStdWString() const
{
    return QString::fromStdString(m_path).toStdWString();
}

const char* path_t::c_str() const
{
    return m_path.c_str();
}

std::string mu::io::suffix(const mu::io::path_t& path)
{
    QFileInfo fi(path.toQString());
    return fi.suffix().toLower().toStdString();
}

mu::io::path_t mu::io::filename(const mu::io::path_t& path, bool includingExtension)
{
    QFileInfo fi(path.toQString());
    return includingExtension ? fi.fileName() : fi.completeBaseName();
}

mu::io::path_t mu::io::basename(const mu::io::path_t& path)
{
    QFileInfo fi(path.toQString());
    return fi.baseName();
}

mu::io::path_t mu::io::completeBasename(const mu::io::path_t& path)
{
    QFileInfo fi(path.toQString());
    return fi.completeBaseName();
}

mu::io::path_t mu::io::absolutePath(const path_t& path)
{
    return QFileInfo(path.toQString()).absolutePath();
}

mu::io::path_t mu::io::dirname(const mu::io::path_t& path)
{
    return QFileInfo(path.toQString()).dir().dirName();
}

mu::io::path_t mu::io::dirpath(const mu::io::path_t& path)
{
    return QFileInfo(path.toQString()).dir().path();
}

mu::io::path_t mu::io::absoluteDirpath(const mu::io::path_t& path)
{
    return QFileInfo(path.toQString()).dir().absolutePath();
}

bool mu::io::isAbsolute(const path_t& path)
{
    return QFileInfo(path.toQString()).isAbsolute();
}

bool mu::io::isAllowedFileName(const path_t& fn_)
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

mu::io::path_t mu::io::escapeFileName(const mu::io::path_t& fn_)
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

paths_t mu::io::pathsFromStrings(const QStringList& list)
{
    paths_t result;

    for (const QString& path : list) {
        result.push_back(path);
    }

    return result;
}

paths_t mu::io::pathsFromString(const std::string& str, const std::string& delim)
{
    if (str.empty()) {
        return {};
    }

    std::vector<std::string> strs;
    strings::split(str, strs, delim);

    paths_t ps;
    ps.reserve(strs.size());
    for (const std::string& s : strs) {
        ps.push_back(path_t(s));
    }
    return ps;
}

std::string mu::io::pathsToString(const paths_t& ps, const std::string& delim)
{
    std::string result;
    bool first = true;
    for (const path_t& _path: ps) {
        if (!first) {
            result += delim;
        }
        first = false;
        result += _path.toStdString();
    }

    return result;
}
