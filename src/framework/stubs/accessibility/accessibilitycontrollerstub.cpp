/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "accessibilitycontrollerstub.h"

using namespace muse::accessibility;

void AccessibilityControllerStub::reg(IAccessible*)
{
}

void AccessibilityControllerStub::unreg(IAccessible*)
{
}

const IAccessible* AccessibilityControllerStub::accessibleRoot() const
{
    return nullptr;
}

const IAccessible* AccessibilityControllerStub::lastFocused() const
{
    return nullptr;
}

bool AccessibilityControllerStub::needToVoicePanelInfo() const
{
    return false;
}

QString AccessibilityControllerStub::currentPanelAccessibleName() const
{
    return QString();
}

void AccessibilityControllerStub::setIgnoreQtAccessibilityEvents(bool)
{
}
