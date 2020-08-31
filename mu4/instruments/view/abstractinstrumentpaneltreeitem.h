//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_INSTRUMENTS_ABSTRACTINSTRUMENTTREEITEM_H
#define MU_INSTRUMENTS_ABSTRACTINSTRUMENTTREEITEM_H

#include <QObject>
#include <QVariant>
#include <QList>

#include "instrumentstypes.h"
#include "notation/inotationparts.h"

namespace mu {
namespace instruments {
class AbstractInstrumentPanelTreeItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)
    Q_PROPERTY(bool isVisible READ isVisible WRITE setIsVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(bool isSelectable READ isSelectable NOTIFY isSelectableChanged)

public:
    explicit AbstractInstrumentPanelTreeItem(const InstrumentTreeItemType::ItemType& type, notation::INotationParts* notationParts,
                                             QObject* parent = nullptr);
    ~AbstractInstrumentPanelTreeItem();

    Q_INVOKABLE virtual bool canAcceptDrop(const int type) const;

    Q_INVOKABLE QString id() const;

    QString title() const;
    int type() const;
    bool isVisible() const;
    bool isSelectable() const;

    AbstractInstrumentPanelTreeItem* parentItem() const;
    void setParentItem(AbstractInstrumentPanelTreeItem* parent);

    QList<AbstractInstrumentPanelTreeItem*> childrenItems() const;
    AbstractInstrumentPanelTreeItem* childAtRow(const int row);
    void appendChild(AbstractInstrumentPanelTreeItem* child);
    void insertChild(AbstractInstrumentPanelTreeItem* child, const int beforeRow);
    virtual void moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                              const int destinationRow);
    virtual void removeChildren(const int row, const int count = 1, const bool deleteChild = false);

    int childCount() const;
    int row() const;

public slots:
    void setType(InstrumentTreeItemType::ItemType type);
    void setTitle(QString title);
    void setIsVisible(bool isVisible);
    void setId(const QString& id);

signals:
    void typeChanged(InstrumentTreeItemType::ItemType type);
    void titleChanged(QString title);
    void isVisibleChanged(bool isVisible);
    void isSelectableChanged(bool isSelectable);

protected:
    notation::INotationParts* notationParts() const;

private:
    QList<AbstractInstrumentPanelTreeItem*> m_children;
    AbstractInstrumentPanelTreeItem* m_parent = nullptr;

    QString m_id = "";
    QString m_title = "";
    InstrumentTreeItemType::ItemType m_type = InstrumentTreeItemType::ItemType::UNDEFINED;
    bool m_isVisible = true;

    notation::INotationParts* m_notationParts = nullptr;
};
}
}

#endif // MU_INSTRUMENTS_ABSTRACTINSTRUMENTTREEITEM_H
