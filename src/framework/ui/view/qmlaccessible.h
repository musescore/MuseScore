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

#ifndef MU_ACCESSIBILITY_QMLACCESSIBLE_H
#define MU_ACCESSIBILITY_QMLACCESSIBLE_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QMap>

#include "accessibility/iaccessible.h"
#include "modularity/ioc.h"
#include "accessibility/iaccessibilitycontroller.h"

namespace mu::ui {
class AccessibleAttached;
class AccessibleItem : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    AccessibleItem(QObject* parent = nullptr);
    virtual ~AccessibleItem() = default;

    AccessibleAttached* accessible() const;
    void setAccessibleParent(AccessibleItem* item);
    void setAccessibleState(accessibility::IAccessible::State st, bool arg);

    virtual size_t accessibleChildCount() const;
    virtual const accessibility::IAccessible* accessibleChild(size_t i) const;

    // QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

    // QObject
    bool event(QEvent* event) override;

private:
    mutable AccessibleAttached* m_attached = nullptr;
};

#define STATE_PROPERTY(P, S) \
    Q_PROPERTY(bool P READ P WRITE set_##P NOTIFY stateChanged FINAL) \
    bool P() const { return m_state.value(S, false); } \
    void set_##P(bool arg) \
    { \
        if (m_state.value(S, false) == arg) \
        return; \
        m_state[S] = arg; \
        emit stateChanged(); \
        onStateChanged(S, arg); \
    } \


class AccessibleAttached : public QObject, public accessibility::IAccessible
{
    Q_OBJECT
    Q_PROPERTY(QmlRole role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QQuickItem * visualItem READ visualItem WRITE setVisualItem NOTIFY visualItemChanged)

    QML_ELEMENT
    QML_ATTACHED(AccessibleAttached)

    INJECT(ui, accessibility::IAccessibilityController, accessibilityController)

public:

    STATE_PROPERTY(selected, State::Selected)

    AccessibleAttached(QObject* object = nullptr);
    ~AccessibleAttached();

    //! NOTE Please sync with accessibility::IAccessible::Role
    enum QmlRole {
        NoRole = 0,
        Application,
        Panel,
        Button,
        RadioButton
    };
    Q_ENUM(QmlRole)

    static AccessibleAttached* qmlAttachedProperties(QObject*);

    void init();
    const accessibility::IAccessible* accessibleRoot() const;
    void notifyAboutParentChanged();
    void setAccessibleState(accessibility::IAccessible::State st, bool arg);

    QmlRole role() const;
    QString name() const;
    QQuickItem* visualItem() const;

    // IAccessible
    const IAccessible* accessibleParent() const override;
    async::Notification accessibleParentChanged() const override;

    size_t accessibleChildCount() const override;
    const IAccessible* accessibleChild(size_t i) const override;

    IAccessible::Role accessibleRole() const override;
    QString accessibleName() const override;
    bool accessibleState(State st) const override;
    QRect accessibleRect() const override;
    QWindow* accessibleWindow() const override;
    // -----

public slots:
    void setRole(QmlRole role);
    void setName(QString name);
    void setVisualItem(QQuickItem* item);

signals:
    void roleChanged(QmlRole role);
    void nameChanged(QString name);
    void visualItemChanged(QQuickItem* item);
    void stateChanged();

private:

    void onStateChanged(State st, bool arg);

    AccessibleItem* m_object = nullptr;
    bool m_registred = false;
    QmlRole m_role = NoRole;
    QString m_name;
    QQuickItem* m_visualItem = nullptr;
    QMap<State, bool> m_state;

    async::Notification m_accessibleParentChanged;
};
}

QML_DECLARE_TYPE(mu::ui::AccessibleAttached)
QML_DECLARE_TYPEINFO(mu::ui::AccessibleAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif // MU_ACCESSIBILITY_QMLACCESSIBLE_H
