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
#include "languagesservicestub.h"

using namespace mu::languages;
using namespace mu;

ValCh<LanguagesHash> LanguagesServiceStub::languages() const
{
    return ValCh<LanguagesHash>();
}

RetCh<LanguageProgress> LanguagesServiceStub::install(const QString&)
{
    RetCh<LanguageProgress> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

RetCh<LanguageProgress> LanguagesServiceStub::update(const QString&)
{
    RetCh<LanguageProgress> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

Ret LanguagesServiceStub::uninstall(const QString&)
{
    return make_ret(Ret::Code::NotSupported);
}

RetVal<Language> LanguagesServiceStub::currentLanguage() const
{
    RetVal<Language> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

Ret LanguagesServiceStub::setCurrentLanguage(const QString&)
{
    return make_ret(Ret::Code::NotSupported);
}

RetCh<Language> LanguagesServiceStub::languageChanged()
{
    RetCh<Language> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}
