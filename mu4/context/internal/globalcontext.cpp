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
#include "globalcontext.h"

using namespace mu::context;
using namespace mu::domain::notation;
using namespace mu::shortcuts;

void GlobalContext::setCurrentNotation(const std::shared_ptr<domain::notation::INotation>& notation)
{
    m_notation = notation;
    m_notationChanged.notify();
}

std::shared_ptr<INotation> GlobalContext::currentNotation() const
{
    return m_notation;
}

mu::async::Notification GlobalContext::currentNotationChanged() const
{
    return m_notationChanged;
}

bool GlobalContext::isPlaying() const
{
    return m_isPlaying;
}

void GlobalContext::setIsPlaying(bool arg)
{
    m_isPlaying = arg;
    m_isPlayingChanged.notify();
}

mu::async::Notification GlobalContext::isPlayingChanged() const
{
    return m_isPlayingChanged;
}

ShortcutContext GlobalContext::currentShortcutContext() const
{
    //! TODO Temporary solution, it does not correctly determine the current context for shortcuts
    if (isPlaying()) {
        return ShortcutContext::Playing;
    } else if (currentNotation()) {
        return ShortcutContext::NotationActive;
    }
    return ShortcutContext::Undefined;
}
