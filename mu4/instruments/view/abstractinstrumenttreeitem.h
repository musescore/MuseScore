#ifndef ABSTRACTINSTRUMENTTREEITEM_H
#define ABSTRACTINSTRUMENTTREEITEM_H

#include <QObject>
#include <QVariant>
#include <QList>

#include "instrumentstypes.h"

namespace mu {
namespace instruments {

class AbstractInstrumentTreeItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)
    Q_PROPERTY(bool isVisible READ isVisible WRITE setIsVisible NOTIFY isVisibleChanged)
    Q_PROPERTY(bool isSelectable READ isSelectable NOTIFY isSelectableChanged)

public:
    explicit AbstractInstrumentTreeItem(const InstrumentTreeItemType::ItemType& type, QObject* parent = nullptr);
    ~AbstractInstrumentTreeItem();

    QString title() const;
    int type() const;
    bool isVisible() const;
    bool isSelectable() const;

    AbstractInstrumentTreeItem* parentItem() const;
    void setParentItem(AbstractInstrumentTreeItem* parent);

    QList<AbstractInstrumentTreeItem*> childrenItems() const;
    AbstractInstrumentTreeItem* childAtRow(const int row);
    void appendChild(AbstractInstrumentTreeItem* child);
    void insertChild(AbstractInstrumentTreeItem* child, const int beforeRow);
    void removeChildren(const int row, const int count = 1, const bool deleteChild = false);

    int childCount() const;
    int row() const;

public slots:
    void setType(InstrumentTreeItemType::ItemType type);
    void setTitle(QString title);
    void setIsVisible(bool isVisible);

signals:
    void typeChanged(InstrumentTreeItemType::ItemType type);
    void titleChanged(QString title);
    void isVisibleChanged(bool isVisible);
    void isSelectableChanged(bool isSelectable);

private:
    QList<AbstractInstrumentTreeItem*> m_children;
    AbstractInstrumentTreeItem* m_parent = nullptr;

    QString m_title = "";
    InstrumentTreeItemType::ItemType m_type = InstrumentTreeItemType::ItemType::UNDEFINED;
    bool m_isVisible = true;
};
}
}

#endif // ABSTRACTINSTRUMENTTREEITEM_H
