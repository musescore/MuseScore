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
#ifndef MU_CONTEXT_GLOBALCONTEXT_H
#define MU_CONTEXT_GLOBALCONTEXT_H

#include <map>
#include "../iglobalcontext.h"

namespace mu {
namespace context {
class GlobalContext : public IGlobalContext
{
public:
    GlobalContext() = default;

    void setCurrentNotation(const std::shared_ptr<domain::notation::INotation>& notation) override;
    std::shared_ptr<domain::notation::INotation> currentNotation() const override;
    async::Notify currentNotationChanged() const override;

private:

    std::shared_ptr<domain::notation::INotation> m_notation;
    async::Notify m_notationChanged;
};
}
}

#endif // MU_CONTEXT_GLOBALCONTEXT_H
