/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include <vector>
#include <map>

#include <QWindow>

#include "ui/internal/navigationcontroller.h"
#include "actions/internal/actionsdispatcher.h"

#include "mocks/navigationmocks.h"
#include "mocks/mainwindowprovidermock.h"
#include "global/tests/mocks/applicationmock.h"

#include "log.h"

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::NiceMock;
using ::testing::_;
using ::testing::SaveArgPointee;
using ::testing::DoAll;

using namespace mu;
using namespace mu::ui;

class NavigationControllerTests : public ::testing::Test
{
public:

    struct Env {
        NavigationController controller;
        std::shared_ptr<actions::IActionsDispatcher> dispatcher;

        //! NOTE Garbage and references
        std::vector<INavigation::Index> idxsRefs;

        Env()
        {
            dispatcher = std::make_shared<actions::ActionsDispatcher>();
            controller.setdispatcher(dispatcher);

            controller.init();

            idxsRefs.reserve(10000);
        }
    };

    INavigation::Index& make_idx(Env& env)
    {
        env.idxsRefs.push_back(INavigation::Index());
        return env.idxsRefs.back();
    }

    struct Control {
        NavigationControlMock* control = nullptr;

        ~Control()
        {
            delete control;
        }
    };

    struct Panel {
        NavigationPanelMock* panel = nullptr;
        std::vector<Control*> controls;
        std::set<INavigationControl*> icontrols;

        ~Panel()
        {
            delete panel;
            for (Control* c : controls) {
                delete c;
            }
        }
    };

    struct Section {
        NavigationSectionMock* section = nullptr;
        std::vector<Panel*> panels;
        std::set<INavigationPanel*> ipanels;

        ~Section()
        {
            delete section;
            for (Panel* p : panels) {
                delete p;
            }
        }
    };

    Control* make_control(INavigation::Index& idx)
    {
        Control* c = new Control();
        c->control = new NiceMock<NavigationControlMock>();
        ON_CALL(*c->control, enabled()).WillByDefault(Return(true));
        ON_CALL(*c->control, active()).WillByDefault(Return(false));
        ON_CALL(*c->control, index()).WillByDefault(ReturnRef(idx));

        return c;
    }

    Panel* make_panel(Env& env, int panelOrder, size_t controlsCount)
    {
        Panel* p = new Panel();

        for (size_t ci = 0; ci < controlsCount; ++ci) {
            INavigation::Index& idx = make_idx(env);
            idx.column = static_cast<int>(ci);

            Control* c = make_control(idx);

            p->controls.push_back(c);
            p->icontrols.insert(c->control);
        }

        p->panel = new NiceMock<NavigationPanelMock>();
        ON_CALL(*p->panel, enabled()).WillByDefault(Return(true));
        ON_CALL(*p->panel, active()).WillByDefault(Return(false));
        ON_CALL(*p->panel, controls()).WillByDefault(ReturnRef(p->icontrols));

        INavigation::Index& idx = make_idx(env);
        idx.setOrder(panelOrder);
        ON_CALL(*p->panel, index()).WillByDefault(ReturnRef(idx));

        return p;
    }

    Section* make_section(Env& env, int sectOrder, size_t panelsCount, size_t controlsCount)
    {
        Section* s = new Section();

        for (size_t pi = 0; pi < panelsCount; ++pi) {
            Panel* p = make_panel(env, static_cast<int>(pi), controlsCount);
            s->panels.push_back(p);
            s->ipanels.insert(p->panel);
        }

        s->section = new NiceMock<NavigationSectionMock>();
        ON_CALL(*s->section, type()).WillByDefault(Return(INavigationSection::Type::Regular));
        ON_CALL(*s->section, enabled()).WillByDefault(Return(true));
        ON_CALL(*s->section, active()).WillByDefault(Return(false));
        ON_CALL(*s->section, panels()).WillByDefault(ReturnRef(s->ipanels));

        INavigation::Index& idx = make_idx(env);
        idx.setOrder(sectOrder);
        ON_CALL(*s->section, index()).WillByDefault(ReturnRef(idx));

        return s;
    }

    void print(Section* s)
    {
        LOGI() << "section: " << s->section->name() << ", idx: " << s->section->index().to_string()
               << ", active: " << s->section->active() << ", enabled: " << s->section->enabled();

        for (const Panel* p : s->panels) {
            LOGI() << "panel: " << p->panel->name() << ", idx: " << p->panel->index().to_string()
                   << ", active: " << p->panel->active() << ", enabled: " << p->panel->enabled();

            for (const Control* c : p->controls) {
                LOGI() << "control: " << c->control->name() << ", idx: " << c->control->index().to_string()
                       << ", active: " << c->control->active() << ", enabled: " << c->control->enabled();
            }
        }
    }
};

TEST_F(NavigationControllerTests, FirstActiveOnNextSection)
{
    Env env;

    //! CASE Nothing active, and we call next section (F6)

    //! GIVEN Two section, not active
    Section* sect1 = make_section(env, 1, 2, 3);
    Section* sect2 = make_section(env, 2, 2, 3);

    env.controller.reg(sect1->section);
    env.controller.reg(sect2->section);

    //! CHECK The first section, the first panel, the first control must be activated.
    EXPECT_CALL(*sect1->section, setActive(true));
    EXPECT_CALL(*sect1->panels[0]->panel, setActive(true));
    EXPECT_CALL(*sect1->panels[0]->controls[0]->control, setActive(true));

    //! CHECK The second section must not be activated
    EXPECT_CALL(*sect2->section, setActive(true)).Times(0);

    //! DO Send action `nav-next-section` (usually F6)
    env.dispatcher->dispatch("nav-next-section");

    delete sect1;
    delete sect2;
}

TEST_F(NavigationControllerTests, FirstActiveOnNextPanel)
{
    Env env;

    //! CASE Nothing active, and we call next panel (Tab)

    //! GIVEN Two section, not active
    Section* sect1 = make_section(env, 1, 2, 3);
    Section* sect2 = make_section(env, 2, 2, 3);

    env.controller.reg(sect1->section);
    env.controller.reg(sect2->section);

    //! CHECK The first section, the first panel, the first control must be activated.
    EXPECT_CALL(*sect1->section, setActive(true));
    EXPECT_CALL(*sect1->panels[0]->panel, setActive(true));
    EXPECT_CALL(*sect1->panels[0]->controls[0]->control, setActive(true));

    //! CHECK The second section must not be activated
    EXPECT_CALL(*sect2->section, setActive(true)).Times(0);

    //! DO Send action `nav-next-section` (usually Tab)
    env.dispatcher->dispatch("nav-next-panel");

    delete sect1;
    delete sect2;
}

TEST_F(NavigationControllerTests, FirstActiveOnPrevSection)
{
    Env env;

    //! CASE Nothing active, and we call prev section (Shift+F6)

    //! GIVEN Two section, not active
    Section* sect1 = make_section(env, 1, 2, 3);
    Section* sect2 = make_section(env, 2, 2, 3);

    env.controller.reg(sect1->section);
    env.controller.reg(sect2->section);

    //! CHECK The last section, the first panel, the first control must be activated.
    EXPECT_CALL(*sect2->section, setActive(true));
    EXPECT_CALL(*sect2->panels[0]->panel, setActive(true));
    EXPECT_CALL(*sect2->panels[0]->controls[0]->control, setActive(true));

    //! CHECK The first section must not be activated
    EXPECT_CALL(*sect1->section, setActive(true)).Times(0);

    //! DO Send action `nav-next-section` (usually Shift+F6)
    env.dispatcher->dispatch("nav-prev-section");

    delete sect1;
    delete sect2;
}

TEST_F(NavigationControllerTests, FirstActiveOnPrevPanel)
{
    Env env;

    //! CASE Nothing active, and we call prev panel (Shift+Tab)

    //! GIVEN Two section, not active
    Section* sect1 = make_section(env, 1, 2, 3);
    Section* sect2 = make_section(env, 2, 2, 3);

    env.controller.reg(sect1->section);
    env.controller.reg(sect2->section);

    //! CHECK The last section, the first panel, the first control must be activated.
    EXPECT_CALL(*sect2->section, setActive(true));
    EXPECT_CALL(*sect2->panels[0]->panel, setActive(true));
    EXPECT_CALL(*sect2->panels[0]->controls[0]->control, setActive(true));

    //! CHECK The first section must not be activated
    EXPECT_CALL(*sect1->section, setActive(true)).Times(0);

    //! DO Send action `nav-next-section` (usually Shift+Tab)
    env.dispatcher->dispatch("nav-prev-panel");

    delete sect1;
    delete sect2;
}
