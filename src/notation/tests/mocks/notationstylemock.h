/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include <gmock/gmock.h>

#include "notation/inotationstyle.h"

namespace mu::notation {
class NotationStyleMock : public INotationStyle
{
public:
    MOCK_METHOD(const PropertyValue&, styleValue, (const StyleId& styleId), (const, override));
    MOCK_METHOD(const PropertyValue&, defaultStyleValue, (const StyleId& styleId), (const, override));
    MOCK_METHOD(void, setStyleValue, (const StyleId& styleId, const PropertyValue& newValue), (override));
    MOCK_METHOD(void, resetStyleValue, (const StyleId& styleId), (override));
    MOCK_METHOD(void, resetStyleValues, (const std::vector<StyleId>& styleIds), (override));

    MOCK_METHOD(bool, canApplyToAllParts, (), (const, override));
    MOCK_METHOD(void, applyToAllParts, (), (override));

    MOCK_METHOD(void, resetAllStyleValues, (const StyleIdSet& exceptTheseOnes), (override));

    MOCK_METHOD(muse::async::Notification, styleChanged, (), (const, override));

    MOCK_METHOD(bool, loadStyle, (const muse::io::path_t&, bool allowAnyVersion), (override));
    MOCK_METHOD(bool, saveStyle, (const muse::io::path_t&), (override));
};
}
