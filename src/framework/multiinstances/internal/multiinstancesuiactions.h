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
#ifndef MUSE_MI_MULTIINSTANCESUIACTIONS_H
#define MUSE_MI_MULTIINSTANCESUIACTIONS_H

#include "ui/iuiactionsmodule.h"

namespace muse::mi {
class MultiInstancesUiActions : public muse::ui::IUiActionsModule
{
public:

    MultiInstancesUiActions() = default;

    const muse::ui::UiActionList& actionsList() const override;
    bool actionEnabled(const muse::ui::UiAction& act) const override;
    async::Channel<muse::actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const muse::ui::UiAction& act) const override;
    async::Channel<muse::actions::ActionCodeList> actionCheckedChanged() const override;

private:
    static const muse::ui::UiActionList m_actions;
};
}

#endif // MUSE_MI_MULTIINSTANCESUIACTIONS_H
