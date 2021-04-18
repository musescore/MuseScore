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
#include "notationreadersregister.h"

using namespace mu::notation;

void NotationReadersRegister::reg(const std::vector<std::string>& suffixes, INotationReaderPtr reader)
{
    for (const std::string& suffix : suffixes) {
        m_readers.insert({ suffix, reader });
    }
}

INotationReaderPtr NotationReadersRegister::reader(const std::string& suffix)
{
    auto it = m_readers.find(suffix);
    if (it != m_readers.end()) {
        return it->second;
    }
    return nullptr;
}
