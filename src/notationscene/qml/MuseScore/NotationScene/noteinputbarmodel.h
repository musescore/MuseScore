/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
#pragma once

#include <QQmlParserStatus>
#include <qqmlintegration.h>

#include "rcommand/commandtypes.h"
#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "rcommand/icommandsregister.h"
#include "context/iglobalcontext.h"
#include "ui/iuistate.h"
#include "notationscene/inotationcommandscontroller.h"
#include "notation/inotationconfiguration.h"
#include "interactive/iinteractive.h"

namespace mu::notation {
class NoteInputBarModel : public muse::uicomponents::AbstractMenuModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus);
    QML_ELEMENT;

    Q_PROPERTY(bool isInputAllowed READ isInputAllowed NOTIFY isInputAllowedChanged)
    Q_PROPERTY(bool isAutomationToolbarActive READ isAutomationToolbarActive NOTIFY isAutomationToolbarActiveChanged)
    Q_PROPERTY(QVariantList automationTypes READ automationTypes CONSTANT)
    Q_PROPERTY(int currentAutomationType READ currentAutomationType WRITE setCurrentAutomationType NOTIFY currentAutomationTypeChanged)
    Q_PROPERTY(bool isVoiceSelectionAvailable READ isVoiceSelectionAvailable NOTIFY currentAutomationTypeChanged)
    Q_PROPERTY(bool isTempoRangeAvailable READ isTempoRangeAvailable NOTIFY currentAutomationTypeChanged)
    //! NOTE: -1 means all voices (UI only for now)
    Q_PROPERTY(int currentAutomationVoice READ currentAutomationVoice WRITE setCurrentAutomationVoice NOTIFY currentAutomationVoiceChanged)
    Q_PROPERTY(int automationVoicesCount READ automationVoicesCount CONSTANT)

    muse::GlobalInject<muse::rcommand::ICommandsRegister> commandsRegister;
    muse::ContextInject<muse::ui::IUiState> uiState = { this };
    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::ContextInject<INotationCommandsController> commandsController = { this };
    muse::GlobalInject<INotationConfiguration> notationConfiguration;
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    explicit NoteInputBarModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isInputAllowed() const;
    bool isAutomationToolbarActive() const;

    QVariantList automationTypes() const;
    int currentAutomationType() const;
    void setCurrentAutomationType(int type);

    bool isVoiceSelectionAvailable() const;
    bool isTempoRangeAvailable() const;

    int currentAutomationVoice() const;
    void setCurrentAutomationVoice(int voice);
    int automationVoicesCount() const;

    Q_INVOKABLE QVariantList automationResetMenuItems();
    Q_INVOKABLE void handleAutomationMenuItem(const QString& itemId);
    Q_INVOKABLE void openTempoRangeDialog();

    static const muse::ui::ToolConfig& defaultNoteInputConfig();

    static const std::string CROSS_STAFF_BEAMING_SUBITEMS;
    static const std::string TUPLET_SUBITEMS;

    struct ServiceItemInfo {
        muse::MnemonicString title;
        muse::ui::IconCode::Code icon = muse::ui::IconCode::Code::NONE;
    };
    static ServiceItemInfo serviceItemInfo(const std::string& intent);

    static bool isItemAvailable(const std::string& intent, AutomationUiVariant variant);

signals:
    void isInputAllowedChanged();
    void isAutomationToolbarActiveChanged();
    void currentAutomationTypeChanged();
    void currentAutomationVoiceChanged();

private:
    enum NoteInputRoles {
        OrderRole = AbstractMenuModel::Roles::UserRole + 1,
        SectionRole
    };

    void classBegin() override;
    void componentComplete() override {}
    void init();
    void load() override;

    bool isToolbarAutomationUi() const;

    muse::uicomponents::MenuItem* makeServiceItem(const std::string& intent, const QString& section);
    muse::uicomponents::MenuItem* makeCommandItem(const muse::rcommand::Command& command, const QString& section);
    muse::uicomponents::MenuItem* makeAddItem(const QString& section);

    muse::uicomponents::MenuItemList makeCrossStaffBeamingItems();
    muse::uicomponents::MenuItemList makeTupletItems();
    muse::uicomponents::MenuItemList makeAddItems();
    muse::uicomponents::MenuItemList makeNotesItems();
    muse::uicomponents::MenuItemList makeIntervalsItems();
    muse::uicomponents::MenuItemList makeMeasuresItems();
    muse::uicomponents::MenuItemList makeFramesItems();
    muse::uicomponents::MenuItemList makeTextItems();
    muse::uicomponents::MenuItemList makeLinesItems();
    muse::uicomponents::MenuItemList makeChordAndFretboardDiagramsItems();

    int m_automationVoice = -1;
    muse::uicomponents::MenuItemList m_automationResetItems;
};
}
