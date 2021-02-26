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
#ifndef MU_AUTOBOT_AUTOBOT_H
#define MU_AUTOBOT_AUTOBOT_H

#include "../iautobot.h"
#include "io/path.h"
#include "async/asyncable.h"

#include "abrunner.h"

namespace mu::autobot {
class Autobot : public IAutobot, public async::Asyncable
{
public:
    Autobot() = default;

    void init();

    void run() override;
    void stop() override;

private:

    void nextScore();

    io::paths m_scores;
    int m_currentIndex = -1;

    bool m_running = false;
    AbRunner m_runner;
};
}

#endif // MU_AUTOBOT_AUTOBOT_H
