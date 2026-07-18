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

#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "ui/iuistate.h"
#include "notationscene/inotationcommandscontroller.h"

namespace mu::notation {
class NoteInputBarModel : public muse::uicomponents::AbstractMenuModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus);
    QML_ELEMENT;

    Q_PROPERTY(bool isInputAllowed READ isInputAllowed NOTIFY isInputAllowedChanged)

    muse::ContextInject<muse::ui::IUiState> uiState = { this };
    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::ContextInject<INotationCommandsController> commandsController = { this };

public:
    explicit NoteInputBarModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool isInputAllowed() const;

signals:
    void isInputAllowedChanged();

private:
    enum NoteInputRoles {
        OrderRole = AbstractMenuModel::Roles::UserRole + 1,
        SectionRole
    };

    void classBegin() override;
    void componentComplete() override {}
    void init();
    void load() override;

    muse::uicomponents::MenuItem* makeActionItem(const muse::ui::UiAction& action, const QString& section,
                                                 const muse::uicomponents::MenuItemList& subitems = {});
    muse::uicomponents::MenuItem* makeAddItem(const QString& section);

    muse::uicomponents::MenuItemList makeCrossStaffBeamingItems();
    muse::uicomponents::MenuItemList makeTupletItems();
    muse::uicomponents::MenuItemList makeAddItems();
    muse::uicomponents::MenuItemList makeNotesItems();
    muse::uicomponents::MenuItemList makeIntervalsItems();
    muse::uicomponents::MenuItemList makeMeasuresItems();
    muse::uicomponents::MenuItemList makeFramesItems();
    muse::uicomponents::MenuItemList makeFramesAppendItems();
    muse::uicomponents::MenuItemList makeTextItems();
    muse::uicomponents::MenuItemList makeLinesItems();
    muse::uicomponents::MenuItemList makeChordAndFretboardDiagramsItems();
};
}
