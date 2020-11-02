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
#ifndef MU_NOTATION_NOTATIONELEMENTS_H
#define MU_NOTATION_NOTATIONELEMENTS_H

#include "inotationelements.h"
#include "igetscore.h"

namespace mu {
namespace notation {
class NotationElements : public INotationElements
{
public:
    NotationElements(IGetScore* getScore);

    Ms::Element* search(const std::string& searchCommand) const override;
    std::vector<Ms::Element*> searchSimilar(ElementPattern* searchPattern) const override;

    Ms::Measure* measure(const int measureIndex) const override;

private:
    Ms::Score* score() const;

    Ms::RehearsalMark* rehearsalMark(const std::string& name) const;
    Ms::Page* page(const int pageIndex) const;

    IGetScore* m_getScore = nullptr;
};
}
}

#endif // MU_NOTATION_NOTATIONELEMENTS_H
