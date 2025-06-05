/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "inotationundostack.h"

#include "igetscore.h"

namespace mu::notation {
class NotationStyle : public INotationStyle
{
public:
    NotationStyle(IGetScore* getScore, INotationUndoStackPtr);

    PropertyValue styleValue(const StyleId& styleId) const override;
    PropertyValue defaultStyleValue(const StyleId& styleId) const override;
    void setStyleValue(const StyleId& styleId, const PropertyValue& newValue) override;
    void resetStyleValue(const StyleId& styleId) override;
    void resetStyleValues(const std::vector<StyleId>& styleIds) override;

    bool canApplyToAllParts() const override;
    void applyToAllParts() override;

    void resetAllStyleValues(const StyleIdSet& exceptTheseOnes = {}) override;

    muse::async::Notification styleChanged() const override;

    bool loadStyle(const muse::io::path_t&, bool allowAnyVersion) override;
    bool saveStyle(const muse::io::path_t&) override;

private:
    mu::engraving::Score* score() const;

    IGetScore* m_getScore = nullptr;
    muse::async::Notification m_styleChanged;
    INotationUndoStackPtr m_undoStack;
};
}

#endif // MU_NOTATION_NOTATIONSTYLE_H
