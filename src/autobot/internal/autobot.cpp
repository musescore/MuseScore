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
#include "autobot.h"

#include <QTimer>

#include "log.h"

#include "abscorelist.h"

using namespace mu::autobot;

void Autobot::init()
{
    m_runner.finished().onReceive(this, [this](const AbContext& ctx) {
        if (ctx.ret) {
            LOGI() << "success finished, score: " << ctx.val<io::path>(AbContext::Key::ScoreFile);
        } else {
            LOGE() << "failed finished, score: " << ctx.val<io::path>(AbContext::Key::ScoreFile);
        }

        QTimer::singleShot(10, [this]() {
            nextScore();
        });
    });

    m_runner.init();
}

void Autobot::run()
{
    LOGI() << "-------";
    RetVal<io::paths> scores = AbScoreList().scoreList();
    if (!scores.ret) {
        LOGE() << "failed get score list, err: " << scores.ret.toString();
        return;
    }

    m_currentIndex = -1;
    m_scores = scores.val;
    m_running = true;

    nextScore();
}

void Autobot::stop()
{
    m_running = false;
}

void Autobot::nextScore()
{
    if (!m_running) {
        return;
    }

    m_currentIndex += 1;
    if (size_t(m_currentIndex) > (m_scores.size() - 1)) {
        return;
    }

    const io::path& score = m_scores.at(size_t(m_currentIndex));
    m_runner.run(score);
}
