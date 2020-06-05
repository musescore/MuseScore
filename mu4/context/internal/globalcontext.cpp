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

void GlobalContext::setNotation(const std::shared_ptr<domain::notation::INotation>& notation)
{
    m_notation = notation;
    m_notationChanged.notify();
}

std::shared_ptr<INotation> GlobalContext::notation() const
{
    return m_notation;
}

deto::async::Notify GlobalContext::notationChanged() const
{
    return m_notationChanged;
}
