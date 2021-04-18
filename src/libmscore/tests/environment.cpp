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

#include "testing/environment.h"

#include "log.h"
#include "framework/fonts/fontsmodule.h"
//#include "notation/notationmodule.h"
//#include "framework/ui/uimodule.h"
//#include "framework/actions/actionsmodule.h"

#include "instruments/instrumentsmodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"

#include "libmscore/score.h"
#include "libmscore/musescoreCore.h"

static mu::testing::SuiteEnvironment libmscore_se(
{
    new mu::fonts::FontsModule(),
//    new mu::ui::UiModule(),
//    new mu::actions::ActionsModule(),
//    new mu::notation::NotationModule()

    new mu::instruments::InstrumentsModule(),
    new mu::uicomponents::UiComponentsModule()
},
    []() {
    LOGI() << "libmscore tests suite post init";
    Ms::MScore::noGui = true;

    new Ms::MuseScoreCore();
    Ms::MScore::init(); // initialize libmscore
}
    );
