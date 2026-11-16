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

#include "notation/imasternotation.h"

namespace mu::notation {
class MasterNotationMock : public IMasterNotation
{
public:
    MOCK_METHOD(project::INotationProject*, project, (), (const, override));

    MOCK_METHOD(muse::Ret, setupNewScore, (engraving::MasterScore * score, const ScoreCreateOptions& options), (override));
    MOCK_METHOD(void, applyOptions,
                (engraving::MasterScore * score, const ScoreCreateOptions& options, bool createdFromTemplate), (override));

    MOCK_METHOD(engraving::MasterScore*, masterScore, (), (const, override));
    MOCK_METHOD(void, setMasterScore, (engraving::MasterScore * masterScore, bool disablePlayback), (override));

    MOCK_METHOD(INotationPtr, notation, (), (override));
    MOCK_METHOD(int, mscVersion, (), (const, override));

    MOCK_METHOD(IExcerptNotationPtr, createEmptyExcerpt, (const QString& name), (const, override));
    MOCK_METHOD(const ExcerptNotationList&, excerpts, (), (const, override));
    MOCK_METHOD(muse::async::Notification, excerptsChanged, (), (const, override));
    MOCK_METHOD(const ExcerptNotationList&, potentialExcerpts, (), (const, override));
    MOCK_METHOD(void, initExcerpts, (const ExcerptNotationList& excerpts), (override));
    MOCK_METHOD(void, setExcerpts, (const ExcerptNotationList& excerpts), (override));
    MOCK_METHOD(void, resetExcerpt, (IExcerptNotationPtr & excerpt), (override));
    MOCK_METHOD(void, sortExcerpts, (ExcerptNotationList & excerpts), (override));
    MOCK_METHOD(void, setExcerptIsOpen, (const INotationPtr excerptNotation, bool opened), (override));

    MOCK_METHOD(INotationPartsPtr, parts, (), (const, override));
    MOCK_METHOD(bool, hasParts, (), (const, override));
    MOCK_METHOD(muse::async::Notification, hasPartsChanged, (), (const, override));

    MOCK_METHOD(INotationPlaybackPtr, playback, (), (const, override));
    MOCK_METHOD(void, initNotationSoloMuteState, (const INotationPtr notation), (override));
    MOCK_METHOD(INotationAutomationPtr, automation, (), (const, override));
};
}
