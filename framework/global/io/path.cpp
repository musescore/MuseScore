//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "path.h"

#include <QFileInfo>
#include <QDir>

#include "stringutils.h"

using namespace mu::io;

path::path(const path& p)
    : m_path(p.m_path)
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

QString path::toQString() const
{
    return QString(m_path);
}

std::string path::toStdString() const
{
    return std::string(m_path.data());
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

std::string mu::io::syffix(const mu::io::path& path)
{
    QFileInfo fi(path.toQString());
    return fi.suffix().toStdString();
}

mu::io::path mu::io::filename(const mu::io::path& path)
{
    QFileInfo fi(path.toQString());
    return fi.fileName();
}

mu::io::path mu::io::basename(const mu::io::path& path)
{
    QFileInfo fi(path.toQString());
    return fi.baseName();
}

mu::io::path mu::io::dirname(const mu::io::path& path)
{
    return QFileInfo(path.toQString()).dir().dirName();
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
    fn = fn.replace(QChar(0xe4), "ae");   // &auml;
    fn = fn.replace(QChar(0xf6), "oe");   // &ouml;
    fn = fn.replace(QChar(0xfc), "ue");   // &uuml;
    fn = fn.replace(QChar(0xdf), "ss");   // &szlig;
    fn = fn.replace(QChar(0xc4), "Ae");   // &Auml;
    fn = fn.replace(QChar(0xd6), "Oe");   // &Ouml;
    fn = fn.replace(QChar(0xdc), "Ue");   // &Uuml;
    fn = fn.replace(QChar(0x266d),"b");   // musical flat sign, happen in instrument names, so can happen in part (file) names
    fn = fn.replace(QChar(0x266f),"#");   // musical sharp sign, can happen in titles, so can happen in score (file) names
    fn = fn.replace(QRegExp("[" + QRegExp::escape("\\/:*?\"<>|") + "]"), "_");         //FAT/NTFS special chars
    return fn;
}
