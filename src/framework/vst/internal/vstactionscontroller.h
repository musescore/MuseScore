/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "global/iinteractive.h"
#include "ui/iinteractiveuriregister.h"
#include "../ivstinstancesregister.h"
#include "../ivstconfiguration.h"

namespace muse::vst {
class VstActionsController : public actions::Actionable
{
    muse::Inject<actions::IActionsDispatcher> dispatcher;
    muse::Inject<IInteractive> interactive;
    muse::Inject<IVstInstancesRegister> instancesRegister;
    muse::Inject<ui::IInteractiveUriRegister> interactiveUriRegister;
    muse::Inject<IVstConfiguration> configuration;

public:
    VstActionsController() = default;

    void init();

    void fxEditor(const actions::ActionQuery& actionQuery);
    void instEditor(const actions::ActionQuery& actionQuery);

    void editorOperation(const std::string& operation, int instanceId);

    void setupUsedView();
    void useView(bool isNew);
    bool isUsedNewView() const;
    bool actionChecked(const actions::ActionCode& act) const;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const;

private:

    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}
