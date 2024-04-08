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
#ifndef MUSE_LANGUAGES_ILANGUAGESSERVICE_H
#define MUSE_LANGUAGES_ILANGUAGESSERVICE_H

#include "modularity/imoduleinterface.h"
#include "async/notification.h"
#include "progress.h"

#include "languagestypes.h"

namespace muse::languages {
class ILanguagesService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILanguagesService)

public:
    virtual ~ILanguagesService() = default;

    virtual const LanguagesHash& languages() const = 0;
    virtual Language language(const QString& languageCode) const = 0;
    virtual const Language& currentLanguage() const = 0;
    virtual async::Notification currentLanguageChanged() const = 0;

    virtual bool hasPlaceholderLanguage() const = 0;
    virtual const Language& placeholderLanguage() const = 0;

    virtual Progress update(const QString& languageCode) = 0;

    virtual bool needRestartToApplyLanguageChange() const = 0;
    virtual async::Channel<bool> needRestartToApplyLanguageChangeChanged() const = 0;
};
}

#endif // MUSE_LANGUAGES_ILANGUAGESSERVICE_H
