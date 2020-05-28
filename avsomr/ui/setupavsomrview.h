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

#ifndef AVS_SETUPAVSOMRVIEW_H
#define AVS_SETUPAVSOMRVIEW_H

#include <memory>
#include "avsomr/avsomr.h"

namespace Ms {
class ScoreView;

namespace Avs {
class SetupAvsOmrView
{
public:
    SetupAvsOmrView();

    void setupView(Ms::ScoreView* view, std::shared_ptr<AvsOmr> avsOmr);
};
} // Avs
} // Ms

#endif // AVS_SETUPAVSOMRVIEW_H
