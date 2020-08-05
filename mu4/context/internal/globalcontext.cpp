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
using namespace mu::async;

static const mu::Uri NOTAION_PAGE("musescore://notation");

void GlobalContext::addMasterNotation(const IMasterNotationPtr& notation)
{
    m_masterNotations.push_back(notation);
}

void GlobalContext::removeMasterNotation(const IMasterNotationPtr& notation)
{
    m_masterNotations.erase(std::remove(m_masterNotations.begin(), m_masterNotations.end(), notation), m_masterNotations.end());
}

const std::vector<IMasterNotationPtr>& GlobalContext::masterNotations() const
{
    return m_masterNotations;
}

bool GlobalContext::containsMasterNotation(const io::path& path) const
{
    for (const auto& n : m_masterNotations) {
        if (n->path() == path) {
            return true;
        }
    }
    return false;
}

void GlobalContext::setCurrentMasterNotation(const IMasterNotationPtr& notation)
{
    m_masterNotation = notation;
    m_masterNotationChanged.notify();
}

IMasterNotationPtr GlobalContext::currentMasterNotation() const
{
    return m_masterNotation;
}

Notification GlobalContext::currentMasterNotationChanged() const
{
    return m_masterNotationChanged;
}

void GlobalContext::setCurrentNotation(const INotationPtr& notation)
{

}

INotationPtr GlobalContext::currentNotation() const
{

}

Notification GlobalContext::currentNotationChanged() const
{

}

ShortcutContext GlobalContext::currentShortcutContext() const
{
    if (playbackController()->isPlaying()) {
        return ShortcutContext::Playing;
    } else if (interactive()->currentUri().val == NOTAION_PAGE) {
        return ShortcutContext::NotationActive;
    }
    return ShortcutContext::Undefined;
}
