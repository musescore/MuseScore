/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <optional>

#include <qqmlintegration.h>

#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "notation/inotationconfiguration.h"
#include "notationscene/inotationcommandscontroller.h"

namespace mu::notation {
//! NOTE: model for the on-score automation widget (the "Widget" automation UI variant)
class AutomationWidgetModel : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isVisible READ isVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(QString currentTypeTitle READ currentTypeTitle NOTIFY currentTypeChanged)

    muse::GlobalInject<INotationConfiguration> notationConfiguration;
    muse::ContextInject<INotationCommandsController> commandsController = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit AutomationWidgetModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;
    Q_INVOKABLE void handleMenuItem(const QString& itemId) override;

    bool isVisible() const;
    QString currentTypeTitle() const;

signals:
    void isVisibleChanged();
    void currentTypeChanged();

private:
    void onCommandStateChanged(const muse::rcommand::Command& command, const muse::rcommand::CommandState& state) override;

    muse::uicomponents::MenuItem* makeTypeItem(engraving::AutomationType type, const std::string& queryTypeParam);
    muse::uicomponents::MenuItemList makeVoiceItems();

    void buildItems();

    bool m_subscribed = false;

    //! NOTE: UI only for now; nullopt means all voices
    std::optional<int> m_voiceIdx;
};
}
