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
#ifndef MU_LANGUAGES_LANGUAGESSERVICESTUB_H
#define MU_LANGUAGES_LANGUAGESSERVICESTUB_H

#include "languages/ilanguagesservice.h"

namespace muse::languages {
class LanguagesServiceStub : public ILanguagesService
{
public:
    const LanguagesHash& languages() const override;
    Language language(const QString& languageCode) const override;
    const Language& currentLanguage() const override;
    async::Notification currentLanguageChanged() const override;

    bool hasPlaceholderLanguage() const override;
    const Language& placeholderLanguage() const override;

    Progress update(const QString& languageCode) override;

    bool restartRequiredToApplyLanguage() const override;
    async::Channel<bool> restartRequiredToApplyLanguageChanged() const override;
};
}

#endif // MU_LANGUAGES_LANGUAGESSERVICESTUB_H
