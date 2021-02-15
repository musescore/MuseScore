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
#include "languagesconfigurationstub.h"

using namespace mu::languages;
using namespace mu;

QString LanguagesConfigurationStub::currentLanguageCode() const
{
    return QString();
}

Ret LanguagesConfigurationStub::setCurrentLanguageCode(const QString&) const
{
    return make_ret(Ret::Code::NotSupported);
}

QUrl LanguagesConfigurationStub::languagesUpdateUrl() const
{
    return QUrl();
}

QUrl LanguagesConfigurationStub::languageFileServerUrl(const QString&) const
{
    return QUrl();
}

ValCh<LanguagesHash> LanguagesConfigurationStub::languages() const
{
    return ValCh<LanguagesHash>();
}

Ret LanguagesConfigurationStub::setLanguages(const LanguagesHash&) const
{
    return make_ret(Ret::Code::NotSupported);
}

io::path LanguagesConfigurationStub::languagesSharePath() const
{
    return io::path();
}

io::path LanguagesConfigurationStub::languagesDataPath() const
{
    return io::path();
}

io::paths LanguagesConfigurationStub::languageFilePaths(const QString&) const
{
    return {};
}

io::path LanguagesConfigurationStub::languageArchivePath(const QString&) const
{
    return io::path();
}
