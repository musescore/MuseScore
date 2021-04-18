//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUTOBOT_ABPAINTPROVIDER_H
#define MU_AUTOBOT_ABPAINTPROVIDER_H

#include <memory>
#include "libmscore/draw/bufferedpaintprovider.h"
#include "libmscore/draw/drawtypes.h"

namespace mu::autobot {
class AbPaintProvider : public draw::BufferedPaintProvider
{
public:

    static const std::shared_ptr<AbPaintProvider>& instance();

    void beginTarget(const std::string& name) override;
    void beforeEndTargetHook(draw::Painter* painter) override;
    bool endTarget(bool endDraw = false) override;

    const draw::DrawData& notationViewDrawData() const;

    void setDiff(const draw::Diff& diff);
    void setIsDiffDrawEnabled(bool arg);

private:
    AbPaintProvider() = default;

    void paintData(draw::IPaintProviderPtr provider, const draw::DrawDataPtr& data, const QColor& overcolor);

    draw::DrawData m_notationViewDrawData;

    draw::Diff m_diff;
    bool m_isDiffDrawEnabled = false;
};
}

#endif // MU_AUTOBOT_ABPAINTPROVIDER_H
