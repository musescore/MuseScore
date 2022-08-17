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
#include "languagesconfigurationstub.h"

using namespace mu::languages;
using namespace mu;

ValCh<QString> LanguagesConfigurationStub::currentLanguageCode() const
{
    return ValCh<QString>();
}

void LanguagesConfigurationStub::setCurrentLanguageCode(const QString&) const
{
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

Ret LanguagesConfigurationStub::setLanguages(const LanguagesHash&)
{
    return make_ret(Ret::Code::NotSupported);
}

io::path_t LanguagesConfigurationStub::languagesUserAppDataPath() const
{
    return io::path_t();
}

io::paths_t LanguagesConfigurationStub::languageFilePaths(const QString&) const
{
    return {};
}

io::path_t LanguagesConfigurationStub::languageArchivePath(const QString&) const
{
    return io::path_t();
}
