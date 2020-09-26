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
#include "abstractplayer.h"

using namespace mu::audio;

AbstractPlayer::~AbstractPlayer() = default;

IPlayer::Status AbstractPlayer::status() const
{
    return m_status;
}

mu::async::Channel<IPlayer::Status> AbstractPlayer::statusChanged() const
{
    return m_statusChanged;
}

void AbstractPlayer::setStatus(const IPlayer::Status& status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;
    m_statusChanged.send(m_status);
}
