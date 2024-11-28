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
#pragma once

#include <gmock/gmock.h>

#include "actions/iactionsdispatcher.h"

namespace muse::actions {
class ActionsDispatcherMock : public IActionsDispatcher
{
public:
    MOCK_METHOD(void, dispatch, (const ActionCode&), (override));
    MOCK_METHOD(void, dispatch, (const ActionCode&, const ActionData&), (override));

    MOCK_METHOD(void, unReg, (Actionable*), (override));
    MOCK_METHOD(void, reg, (Actionable*, const ActionCode&, const ActionCallBackWithNameAndData&), (override));
    MOCK_METHOD(bool, isReg, (Actionable*), (const, override));
    MOCK_METHOD(ActionCodeList, actionList, (), (const, override));
};
}
