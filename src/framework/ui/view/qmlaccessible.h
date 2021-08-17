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

#ifndef MU_UI_QMLACCESSIBLE_H
#define MU_UI_QMLACCESSIBLE_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QMap>

#include "accessibility/iaccessible.h"
#include "modularity/ioc.h"
#include "accessibility/iaccessibilitycontroller.h"

#define STATE_PROPERTY(P, S) \
    Q_PROPERTY(bool P READ P WRITE set_##P NOTIFY stateChanged FINAL) \
    bool P() const { return m_state.value(S, false); } \
    void set_##P(bool arg) { setState(S, arg); } \

namespace mu::ui {
class MUAccessible
{
    Q_GADGET
public:
    MUAccessible() = default;

    //! NOTE Please sync with accessibility::IAccessible::Role (src/framework/accessibility/iaccessible.h)
    enum Role {
        NoRole = 0,
        Application,
        Dialog,
        Panel,
        StaticText,
        EditableText,
        Button,
        CheckBox,
        RadioButton,
        ComboBox,
        ListItem,
        Information
    };
    Q_ENUM(Role)
};

class AccessibleItem : public QObject, public QQmlParserStatus, public accessibility::IAccessible
{
    Q_OBJECT

    Q_PROPERTY(AccessibleItem * accessibleParent READ accessibleParent_property WRITE setAccessibleParent NOTIFY accessiblePrnChanged)
    Q_PROPERTY(mu::ui::MUAccessible::Role role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool ignored READ ignored WRITE setIgnored NOTIFY ignoredChanged)
    Q_PROPERTY(QQuickItem * visualItem READ visualItem WRITE setVisualItem NOTIFY visualItemChanged)

    Q_INTERFACES(QQmlParserStatus)

    INJECT(ui, accessibility::IAccessibilityController, accessibilityController)

public:

    STATE_PROPERTY(selected, State::Selected)
    STATE_PROPERTY(focused, State::Focused)

    AccessibleItem(QObject* parent = nullptr);
    ~AccessibleItem();

    MUAccessible::Role role() const;
    QString name() const;
    QString description() const;
    bool ignored() const;
    QQuickItem* visualItem() const;

    const IAccessible* accessibleRoot() const;
    void setState(State st, bool arg);

    AccessibleItem* accessibleParent_property() const;
    void setAccessibleParent(AccessibleItem* p);
    void addChild(AccessibleItem* item);
    void removeChild(AccessibleItem* item);

    // IAccessible
    const IAccessible* accessibleParent() const override;
    size_t accessibleChildCount() const override;
    const IAccessible* accessibleChild(size_t i) const override;

    IAccessible::Role accessibleRole() const override;
    QString accessibleName() const override;
    QString accessibleDescription() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;

    async::Channel<Property> accessiblePropertyChanged() const override;
    async::Channel<State, bool> accessibleStateChanged() const override;
    // -----

    // QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

public slots:
    void setRole(MUAccessible::Role role);
    void setName(QString name);
    void setDescription(QString description);
    void setIgnored(bool ignored);
    void setVisualItem(QQuickItem* item);

signals:
    void accessiblePrnChanged();
    void roleChanged(MUAccessible::Role role);
    void nameChanged(QString name);
    void descriptionChanged(QString description);
    void ignoredChanged(bool ignored);
    void visualItemChanged(QQuickItem* item);
    void stateChanged();

private:

    QQuickItem* resolveVisualItem() const;

    bool m_registred = false;
    AccessibleItem* m_accessibleParent = nullptr;
    QList<AccessibleItem*> m_children;
    MUAccessible::Role m_role = MUAccessible::NoRole;
    QString m_name;
    QString m_description;
    bool m_ignored = false;
    QQuickItem* m_visualItem = nullptr;
    QMap<State, bool> m_state;
    async::Channel<Property> m_accessiblePropertyChanged;
    async::Channel<State, bool> m_accessibleStateChanged;
};
}

#endif // MU_UI_QMLACCESSIBLE_H
