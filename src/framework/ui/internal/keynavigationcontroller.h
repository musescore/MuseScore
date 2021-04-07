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

namespace mu::ui {
class KeyNavigationController : public IKeyNavigationController, public actions::Actionable, public async::Asyncable
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
    void nextSubSection();
    void prevSubSection();
    void nextControl();
    void prevControl();

    void triggerControl();
    void onForceActiveRequested(IKeyNavigationSection* sec, IKeyNavigationSubSection* sub, IKeyNavigationControl* ctrl);

    void activateSection(IKeyNavigationSection* s);
    void deactivateSection(IKeyNavigationSection* s);
    void activateSubSection(IKeyNavigationSubSection* s);
    void deactivateSubSection(IKeyNavigationSubSection* s);

    const QList<IKeyNavigationSubSection*>& subsectionsOfActiveSection(bool doActiveIfNoAnyActive = true);
    const QList<IKeyNavigationControl*>& controlsOfActiveSubSection(bool doActiveIfNoAnyActive = false);

    QList<IKeyNavigationSection*> m_sections;
};
}

#endif // MU_UI_KEYNAVIGATIONCONTROLLER_H
