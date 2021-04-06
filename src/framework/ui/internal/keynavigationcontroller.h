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
#ifndef MU_UI_KEYNAVIGATIONCONTROLLER_H
#define MU_UI_KEYNAVIGATIONCONTROLLER_H

#include <vector>

#include "../ikeynavigationcontroller.h"
#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"

namespace mu::ui {
class KeyNavigationController : public IKeyNavigationController, public actions::Actionable
{
    INJECT(ui, actions::IActionsDispatcher, dispatcher)
public:
    KeyNavigationController() = default;

    void reg(IKeyNavigationSection* s) override;
    void unreg(IKeyNavigationSection* s) override;

    void init();

private:
    void nextSection();
    void prevSection();

    using Chain = std::vector<IKeyNavigationSection*>;

    IKeyNavigationSection* firstSection() const;
    IKeyNavigationSection* lastSection() const;
    IKeyNavigationSection* nextSection(const IKeyNavigationSection* s) const;
    IKeyNavigationSection* prevSection(const IKeyNavigationSection* s) const;
    IKeyNavigationSection* findFirstEnabledSection(Chain::const_iterator from, Chain::const_iterator end) const;
    IKeyNavigationSection* findLastEnabledSection(Chain::const_iterator from, Chain::const_iterator begin) const;
    IKeyNavigationSection* activeSection() const;

    Chain m_chain;
};
}

#endif // MU_UI_KEYNAVIGATIONCONTROLLER_H
