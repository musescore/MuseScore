//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_NOTATION_SCORECALLBACKS_H
#define MU_NOTATION_SCORECALLBACKS_H

#include "libmscore/mscoreview.h"
#include "libmscore/musescoreCore.h"

class QRectF;
class QRect;

namespace mu::notation {
class ScoreCallbacks : public Ms::MuseScoreView, public Ms::MuseScoreCore
{
public:
    ScoreCallbacks() = default;

    void dataChanged(const QRectF&) override;
    void updateAll() override;
    void drawBackground(mu::draw::Painter*, const QRectF&) const override;
    const QRect geometry() const override;
};
}

#endif // MU_NOTATION_SCORECALLBACKS_H
