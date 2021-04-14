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

#include <QList>

#include "../ikeynavigationcontroller.h"
#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "async/asyncable.h"
#include "global/iinteractive.h"

namespace mu::ui {
class KeyNavigationController : public IKeyNavigationController, public actions::Actionable, public async::Asyncable
{
    INJECT(ui, actions::IActionsDispatcher, dispatcher)
    INJECT(ui, framework::IInteractive, interactive)

public:
    KeyNavigationController() = default;

    enum MoveDirection {
        First = 0,
        Last,
        Right,
        Left,
        Up,
        Down
    };

    void reg(IKeyNavigationSection* s) override;
    void unreg(IKeyNavigationSection* s) override;

    const std::set<IKeyNavigationSection*>& sections() const override;

    void init();

private:

    void devShowControls();

    void goToNextSection();
    void goToPrevSection();
    void goToNextSubSection();
    void goToPrevSubSection();

    void goToFirstControl();
    void goToLastControl();
    void goToNextRowControl();
    void goToPrevRowControl();

    void goToControl(MoveDirection direction, IKeyNavigationSubSection* activeSubSec = nullptr);

    void onLeft();
    void onRight();
    void onUp();
    void onDown();

    void doTriggerControl();
    void onForceActiveRequested(IKeyNavigationSection* sec, IKeyNavigationSubSection* sub, IKeyNavigationControl* ctrl);

    void doActivateSection(IKeyNavigationSection* s);
    void doDeactivateSection(IKeyNavigationSection* s);
    void doActivateSubSection(IKeyNavigationSubSection* s);
    void doDeactivateSubSection(IKeyNavigationSubSection* s);
    void doActivateControl(IKeyNavigationControl* c);
    void doDeactivateControl(IKeyNavigationControl* c);

    void doActivateFirst();
    void doActivateLast();

    IKeyNavigationSection* activeSection() const;
    IKeyNavigationSubSection* activeSubSection() const;
    IKeyNavigationControl* activeControl() const;

    std::set<IKeyNavigationSection*> m_sections;
};
}

#endif // MU_UI_KEYNAVIGATIONCONTROLLER_H
