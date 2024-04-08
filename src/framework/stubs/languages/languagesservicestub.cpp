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
#include "languagesservicestub.h"

using namespace muse::languages;
using namespace muse;

const LanguagesHash& LanguagesServiceStub::languages() const
{
    static const LanguagesHash lh;
    return lh;
}

Language LanguagesServiceStub::language(const QString&) const
{
    return {};
}

const Language& LanguagesServiceStub::currentLanguage() const
{
    static const Language cl;
    return cl;
}

async::Notification LanguagesServiceStub::currentLanguageChanged() const
{
    return async::Notification();
}

bool LanguagesServiceStub::hasPlaceholderLanguage() const
{
    return false;
}

const Language& LanguagesServiceStub::placeholderLanguage() const
{
    static const Language pl;
    return pl;
}

Progress LanguagesServiceStub::update(const QString&)
{
    return Progress();
}

bool LanguagesServiceStub::needRestartToApplyLanguageChange() const
{
    return false;
}

async::Channel<bool> LanguagesServiceStub::needRestartToApplyLanguageChangeChanged() const
{
    return async::Channel<bool>();
}
