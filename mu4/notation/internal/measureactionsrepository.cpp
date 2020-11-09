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
#include "measureactionsrepository.h"

using namespace mu::notation;
using namespace mu::actions;

ActionList MeasureActionsRepository::actions() const
{
    ActionList actions;
    actions.push_back(actionRegister()->action("cut"));
    actions.push_back(actionRegister()->action("copy"));
    actions.push_back(actionRegister()->action("paste"));
    actions.push_back(actionRegister()->action("swap"));
    actions.push_back(actionRegister()->action("delete"));
    actions.push_back(actionRegister()->action("staff-properties"));

    return actions;
}
