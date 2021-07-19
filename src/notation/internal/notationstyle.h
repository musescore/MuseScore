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

#ifndef MU_NOTATION_NOTATIONSTYLE_H
#define MU_NOTATION_NOTATIONSTYLE_H

#include "inotationstyle.h"

#include "igetscore.h"

namespace mu::notation {
class NotationStyle : public INotationStyle
{
public:
    NotationStyle(IGetScore* getScore);

    QVariant styleValue(const StyleId& styleId) const override;
    QVariant defaultStyleValue(const StyleId& styleId) const override;
    void setStyleValue(const StyleId& styleId, const QVariant& newValue) override;
    void resetStyleValue(const StyleId& styleId) override;

    bool canApplyToAllParts() const override;
    void applyToAllParts() override;

    async::Notification styleChanged() const override;

private:
    IGetScore* m_getScore = nullptr;
    async::Notification m_styleChanged;
};
}

#endif // MU_NOTATION_NOTATIONSTYLE_H
