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
#ifndef MUSE_AUTOBOT_ABPAINTPROVIDER_H
#define MUSE_AUTOBOT_ABPAINTPROVIDER_H

#include <memory>

#include "draw/bufferedpaintprovider.h"

namespace muse::autobot {
class AbPaintProvider : public draw::BufferedPaintProvider
{
public:

    static const std::shared_ptr<AbPaintProvider>& instance();

    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(draw::Painter* painter) override;
    bool endTarget(bool endDraw = false) override;

    const muse::draw::DrawDataPtr& notationViewDrawData() const;

    void setDiff(const muse::draw::Diff& diff);
    void setIsDiffDrawEnabled(bool arg);

private:
    AbPaintProvider() = default;

    void paintData(muse::draw::IPaintProviderPtr provider, const muse::draw::DrawDataPtr& data, const QColor& overcolor);

    muse::draw::DrawDataPtr m_notationViewDrawData;

    muse::draw::Diff m_diff;
    bool m_isDiffDrawEnabled = false;
};
}

#endif // MUSE_AUTOBOT_ABPAINTPROVIDER_H
