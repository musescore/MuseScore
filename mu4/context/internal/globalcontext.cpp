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

static const mu::Uri NOTAION_PAGE("musescore://notation");

void GlobalContext::addNotation(const std::shared_ptr<domain::notation::INotation>& notation)
{
    m_notations.push_back(notation);
}

void GlobalContext::removeNotation(const std::shared_ptr<domain::notation::INotation>& notation)
{
    m_notations.erase(std::remove(m_notations.begin(), m_notations.end(), notation), m_notations.end());
}

const std::vector<std::shared_ptr<mu::domain::notation::INotation> >& GlobalContext::notations() const
{
    return m_notations;
}

bool GlobalContext::containsNotation(const io::path& path) const
{
    for (const auto& n : m_notations) {
        if (n->path() == path) {
            return true;
        }
    }
    return false;
}

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

ShortcutContext GlobalContext::currentShortcutContext() const
{
    if (playbackController()->isPlaying()) {
        return ShortcutContext::Playing;
    } else if (launcher()->currentUri().val == NOTAION_PAGE) {
        return ShortcutContext::NotationActive;
    }
    return ShortcutContext::Undefined;
}
