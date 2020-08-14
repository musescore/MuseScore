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
#include "stringutils.h"

#include <QFileInfo>

#ifndef NO_QT_SUPPORT
mu::io::path mu::io::pathFromQString(const QString& s)
{
    return s.toStdString();
}

QString mu::io::pathToQString(const path& p)
{
    return QString::fromStdString(p);
}

#endif

mu::io::path mu::io::syffix(const mu::io::path& path)
{
    auto pos = path.find_last_of(".");
    if (pos == std::string::npos) {
        return std::string();
    }
    std::string sfx = path.substr(pos + 1);
    return strings::toLower(sfx);
}

std::string mu::io::filename(const path& path)
{
    QFileInfo fi(pathToQString(path));
    return fi.fileName().toStdString();
}

std::string mu::io::basename(const mu::io::path& path)
{
    QFileInfo fi(pathToQString(path));
    return fi.baseName().toStdString();
}

QString mu::io::escapeFileName(QString fn)
{
    //
    // special characters in filenames are a constant source
    // of trouble, this replaces some of them common in german:
    //
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
