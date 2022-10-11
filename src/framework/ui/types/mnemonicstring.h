/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#ifndef MU_UI_MNEMONICSTRING_H
#define MU_UI_MNEMONICSTRING_H

#include <QString>

#include "types/translatablestring.h"

namespace mu::ui {
class MnemonicString
{
public:
    MnemonicString() = default;
    MnemonicString(const TranslatableString& raw)
        : m_raw(raw) {}

    inline bool isEmpty() const
    {
        return m_raw.isEmpty();
    }

    inline const TranslatableString& raw() const
    {
        return m_raw;
    }

    inline QString qTranslatedWithMnemonicAmpersand() const
    {
        return m_raw.qTranslated();
    }

    QString qTranslatedWithMnemonicUnderline() const;

    QString qTranslatedWithoutMnemonic() const;

    inline bool operator ==(const MnemonicString& other) const
    {
        return m_raw == other.m_raw;
    }

    inline bool operator !=(const MnemonicString& other) const
    {
        return m_raw != other.m_raw;
    }

private:
    TranslatableString m_raw;
};
}

#endif // MU_UI_MNEMONICSTRING_H
