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
#ifndef MU_ENGRAVING_ENGRAVINGCONFIGURATIONMOCK_H
#define MU_ENGRAVING_ENGRAVINGCONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "engraving/iengravingconfiguration.h"

namespace mu::engraving {
class EngravingConfigurationMock : public IEngravingConfiguration
{
public:
    MOCK_METHOD(io::path_t, appDataPath, (), (const, override));

    MOCK_METHOD(io::path_t, defaultStyleFilePath, (), (const, override));
    MOCK_METHOD(void, setDefaultStyleFilePath, (const io::path_t&), (override));

    MOCK_METHOD(io::path_t, partStyleFilePath, (), (const, override));
    MOCK_METHOD(void, setPartStyleFilePath, (const io::path_t&), (override));

    MOCK_METHOD(SizeF, defaultPageSize, (), (const, override));

    MOCK_METHOD(String, iconsFontFamily, (), (const, override));

    MOCK_METHOD(draw::Color, defaultColor, (), (const, override));
    MOCK_METHOD(draw::Color, scoreInversionColor, (), (const, override));
    MOCK_METHOD(draw::Color, invisibleColor, (), (const, override));
    MOCK_METHOD(draw::Color, lassoColor, (), (const, override));
    MOCK_METHOD(draw::Color, warningColor, (), (const, override));
    MOCK_METHOD(draw::Color, warningSelectedColor, (), (const, override));
    MOCK_METHOD(draw::Color, criticalColor, (), (const, override));
    MOCK_METHOD(draw::Color, criticalSelectedColor, (), (const, override));
    MOCK_METHOD(draw::Color, formattingMarksColor, (), (const, override));
    MOCK_METHOD(draw::Color, thumbnailBackgroundColor, (), (const, override));
    MOCK_METHOD(draw::Color, noteBackgroundColor, (), (const, override));

    MOCK_METHOD(double, guiScaling, (), (const, override));

    MOCK_METHOD(draw::Color, selectionColor, (engraving::voice_idx_t, bool), (const, override));
    MOCK_METHOD(void, setSelectionColor, (engraving::voice_idx_t, draw::Color), (override));
    MOCK_METHOD((async::Channel<engraving::voice_idx_t, draw::Color>), selectionColorChanged, (), (const, override));

    MOCK_METHOD(bool, scoreInversionEnabled, (), (const, override));
    MOCK_METHOD(void, setScoreInversionEnabled, (bool), (override));
    MOCK_METHOD(async::Notification, scoreInversionChanged, (), (const, override));

    MOCK_METHOD(draw::Color, highlightSelectionColor, (engraving::voice_idx_t), (const, override));

    MOCK_METHOD(const DebuggingOptions&, debuggingOptions, (), (const, override));
    MOCK_METHOD(void, setDebuggingOptions, (const DebuggingOptions&), (override));
    MOCK_METHOD(async::Notification, debuggingOptionsChanged, (), (const, override));

    MOCK_METHOD(bool, isAccessibleEnabled, (), (const, override));

    MOCK_METHOD(bool, guitarProImportExperimental, (), (const, override));
    MOCK_METHOD(bool, negativeFretsAllowed, (), (const, override));
    MOCK_METHOD(bool, tablatureParenthesesZIndexWorkaround, (), (const, override));
    MOCK_METHOD(bool, crossNoteHeadAlwaysBlack, (), (const, override));
    MOCK_METHOD(bool, enableExperimentalFretCircle, (), (const, override));
    MOCK_METHOD(void, setGuitarProMultivoiceEnabled, (bool), (override));
    MOCK_METHOD(bool, guitarProMultivoiceEnabled, (), (const, override));
};
}

#endif // MU_ENGRAVING_ENGRAVINGCONFIGURATIONMOCK_H
