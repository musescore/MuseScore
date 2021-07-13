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
#ifndef MU_INSTRUMENTSSCENE_ABSTRACTINSTRUMENTTREEITEM_H
#define MU_INSTRUMENTSSCENE_ABSTRACTINSTRUMENTTREEITEM_H

#include <QObject>
#include <QVariant>
#include <QList>

#include "instrumentsscenetypes.h"
#include "notation/inotationparts.h"

namespace mu::instrumentsscene {
class AbstractInstrumentsPanelTreeItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)
    Q_PROPERTY(bool canChangeVisibility READ canChangeVisibility NOTIFY canChangeVisibilityChanged)
    Q_PROPERTY(bool isVisible READ isVisible WRITE setIsVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(bool isSelectable READ isSelectable NOTIFY isSelectableChanged)

public:
    explicit AbstractInstrumentsPanelTreeItem(const InstrumentsTreeItemType::ItemType& type, notation::INotationPartsPtr notationParts,
                                              QObject* parent = nullptr);
    virtual ~AbstractInstrumentsPanelTreeItem();

    Q_INVOKABLE virtual bool canAcceptDrop(const int type) const;
    Q_INVOKABLE virtual void appendNewItem();

    Q_INVOKABLE QString id() const;

    QString title() const;
    int type() const;
    bool isVisible() const;
    bool isSelectable() const;

    bool canChangeVisibility() const;
    void setCanChangeVisibility(bool value);

    AbstractInstrumentsPanelTreeItem* parentItem() const;
    void setParentItem(AbstractInstrumentsPanelTreeItem* parent);

    QList<AbstractInstrumentsPanelTreeItem*> childrenItems() const;
    bool isEmpty() const;

    AbstractInstrumentsPanelTreeItem* childAtId(const QString& id) const;
    AbstractInstrumentsPanelTreeItem* childAtRow(const int row) const;

    void appendChild(AbstractInstrumentsPanelTreeItem* child);
    void insertChild(AbstractInstrumentsPanelTreeItem* child, const int beforeRow);
    void replaceChild(AbstractInstrumentsPanelTreeItem* child, const int row);

    virtual void moveChildren(const int sourceRow, const int count, AbstractInstrumentsPanelTreeItem* destinationParent,
                              const int destinationRow);
    virtual void removeChildren(const int row, const int count = 1, const bool deleteChild = false);

    int childCount() const;
    int row() const;

public slots:
    void setType(InstrumentsTreeItemType::ItemType type);
    void setTitle(QString title);
    void setIsVisible(bool isVisible);
    void setId(const QString& id);

signals:
    void typeChanged(InstrumentsTreeItemType::ItemType type);
    void titleChanged(QString title);
    void canChangeVisibilityChanged(bool canChange);
    void isVisibleChanged(bool isVisible);
    void isSelectableChanged(bool isSelectable);

protected:
    notation::INotationPartsPtr notationParts() const;

private:
    QList<AbstractInstrumentsPanelTreeItem*> m_children;
    AbstractInstrumentsPanelTreeItem* m_parent = nullptr;

    QString m_id;
    QString m_title;
    InstrumentsTreeItemType::ItemType m_type = InstrumentsTreeItemType::ItemType::UNDEFINED;
    bool m_isVisible = false;
    bool m_canChangeVisibility = false;

    notation::INotationPartsPtr m_notationParts = nullptr;
};
}

#endif // MU_INSTRUMENTSSCENE_ABSTRACTINSTRUMENTTREEITEM_H
