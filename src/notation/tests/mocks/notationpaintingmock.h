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

#include "notation/inotationpainting.h"

namespace mu::notation {
class NotationPaintingMock : public INotationPainting
{
public:
    MOCK_METHOD(void, setViewMode, (const ViewMode& vm), (override));
    MOCK_METHOD(ViewMode, viewMode, (), (const, override));
    MOCK_METHOD(muse::async::Notification, viewModeChanged, (), (const, override));

    MOCK_METHOD(int, pageCount, (), (const, override));
    MOCK_METHOD(muse::SizeF, pageSizeInch, (), (const, override));
    MOCK_METHOD(muse::SizeF, pageSizeInch, (const Options& opt), (const, override));

    MOCK_METHOD(void, paintView,
                (muse::draw::Painter * painter, const muse::RectF& frameRect, bool isPrinting, bool isAutomation,
                 bool showPlaybackInfluence),
                (override));
    MOCK_METHOD(void, paintPdf, (muse::draw::Painter * painter, const Options& opt), (override));
    MOCK_METHOD(void, paintPrint, (muse::draw::Painter * painter, const Options& opt), (override));
    MOCK_METHOD(void, paintPng, (muse::draw::Painter * painter, const Options& opt), (override));
};
}
