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

#include <QObject>
#include <QString>

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

#include "ui/uiaction.h"
#include "uicomponents/view/menuitem.h"

#include "abstracttoolbarmodel.h"

namespace muse::uicomponents {
class ToolBarItem : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id NOTIFY idChanged)

    Q_PROPERTY(QString code READ code_property NOTIFY actionChanged)
    Q_PROPERTY(QString shortcuts READ shortcutsTitle NOTIFY actionChanged)

    Q_PROPERTY(QString title READ translatedTitle NOTIFY actionChanged)
    Q_PROPERTY(bool showTitle READ showTitle WRITE setShowTitle NOTIFY showTitleChanged)
    Q_PROPERTY(bool isTitleBold READ isTitleBold WRITE setIsTitleBold NOTIFY isTitleBoldChanged)

    Q_PROPERTY(QString description READ description_property NOTIFY actionChanged)

    Q_PROPERTY(int icon READ icon_property NOTIFY actionChanged)

    Q_PROPERTY(bool enabled READ enabled_property NOTIFY stateChanged)

    Q_PROPERTY(bool checkable READ checkable_property NOTIFY actionChanged)
    Q_PROPERTY(bool checked READ checked_property NOTIFY stateChanged)

    Q_PROPERTY(bool selected READ selected_property NOTIFY selectedChanged)

    Q_PROPERTY(bool isTransparent READ isTransparent WRITE setIsTransparent NOTIFY isTransparentChanged)

    Q_PROPERTY(int type READ type_property NOTIFY typeChanged)

    Q_PROPERTY(QList<MenuItem*> menuItems READ menuItems NOTIFY menuItemsChanged)
    Q_PROPERTY(bool isMenuSecondary READ isMenuSecondary NOTIFY isMenuSecondaryChanged)

    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    ToolBarItem(QObject* parent = nullptr);
    ToolBarItem(const ui::UiAction& action, ToolBarItemType::Type type, QObject* parent = nullptr);

    Q_INVOKABLE void activate();
    Q_INVOKABLE void handleMenuItem(const QString& menuId);

    QString id() const;
    QString translatedTitle() const;

    bool selected() const;

    ToolBarItemType::Type type() const;

    QList<MenuItem*> menuItems() const;

    bool isMenuSecondary() const;
    void setIsMenuSecondary(bool secondary);

    ui::UiAction action() const;
    ui::UiActionState state() const;
    muse::actions::ActionData args() const;

    bool isValid() const;

    QString shortcutsTitle() const;

    bool showTitle() const;
    void setShowTitle(bool show);

    bool isTransparent() const;
    void setIsTransparent(bool isTransparent);

    bool isTitleBold() const;
    void setIsTitleBold(bool newIsTitleBold);

public slots:
    void setId(const QString& id);
    void setTitle(const TranslatableString& title);
    void setState(const ui::UiActionState& state);
    void setSelected(bool selected);
    void setType(muse::uicomponents::ToolBarItemType::Type type);
    void setMenuItems(const QList<uicomponents::MenuItem*>& menuItems);
    void setAction(const ui::UiAction& action);
    void setArgs(const muse::actions::ActionData& args);

signals:
    void idChanged(QString id);
    void titleChanged(QString title);
    void stateChanged();
    void selectedChanged(bool selected);
    void typeChanged(int type);
    void menuItemsChanged(QList<uicomponents::MenuItem*> menuItems, const QString& menuId);
    void isMenuSecondaryChanged(bool secondary);
    void actionChanged();

    void showTitleChanged();
    void isTitleBoldChanged();
    void isTransparentChanged();

private:
    QString code_property() const;
    QString description_property() const;
    int icon_property() const;
    bool enabled_property() const;
    bool checkable_property() const;
    bool checked_property() const;
    bool selected_property() const;
    int type_property() const;

    QString m_id;
    ui::UiActionState m_state;
    bool m_selected = false;
    ToolBarItemType::Type m_type = ToolBarItemType::ACTION;
    muse::actions::ActionData m_args;
    QList<MenuItem*> m_menuItems;
    bool m_isMenuSecondary = false;
    bool m_showTitle = false;
    bool m_isTitleBold = false;

    ui::UiAction m_action;
    bool m_isTransparent = true;
};
using ToolBarItemList = QList<ToolBarItem*>;
}
