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
#ifndef MUSE_AUTOBOT_AUTOBOTACTIONS_H
#define MUSE_AUTOBOT_AUTOBOTACTIONS_H

#include "ui/iuiactionsmodule.h"

namespace muse::autobot {
class AutobotActions : public mu::ui::IUiActionsModule
{
public:
    AutobotActions() = default;

    const mu::ui::UiActionList& actionsList() const override;
    bool actionEnabled(const mu::ui::UiAction& act) const override;
    async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const mu::ui::UiAction& act) const override;
    async::Channel<muse::actions::ActionCodeList> actionCheckedChanged() const override;

private:
    static const mu::ui::UiActionList m_actions;
};
}

#endif // MUSE_AUTOBOT_AUTOBOTACTIONS_H
