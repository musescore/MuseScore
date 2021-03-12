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
#include "menucontrollersregister.h"

using namespace mu::uicomponents;

void MenuControllersRegister::registerController(MenuType menuType, IMenuControllerPtr controller)
{
    m_controllers[menuType] = controller;
}

IMenuControllerPtr MenuControllersRegister::controller(MenuType menuType) const
{
    auto it = m_controllers.find(menuType);
    if (it != m_controllers.end()) {
        return it->second;
    }

    return nullptr;
}

IMenuControllerPtrList MenuControllersRegister::controllers() const
{
    IMenuControllerPtrList result;

    for (auto it = m_controllers.begin(); it != m_controllers.end(); ++it) {
        result.push_back(it->second);
    }

    return result;
}
