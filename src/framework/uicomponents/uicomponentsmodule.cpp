/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "uicomponentsmodule.h"

#include "modularity/ioc.h"

#include "interactive/iinteractiveuriregister.h"

using namespace muse::uicomponents;
using namespace muse::modularity;

std::string UiComponentsModule::moduleName() const
{
    return "uicomponents";
}

void UiComponentsModule::resolveImports()
{
    auto ir = globalIoc()->resolve<interactive::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://interactive/selectmultipledirectories"), "Muse.UiComponents", "SelectMultipleDirectoriesDialog");
    }
}
