#include "abstractinstrumenttreeitem.h"

using namespace mu::scene::instruments;

AbstractInstrumentTreeItem::AbstractInstrumentTreeItem(const InstrumentTreeItemType::ItemType& type, QObject* parent)
    : QObject(parent)
{
    setType(type);
}

AbstractInstrumentTreeItem::~AbstractInstrumentTreeItem()
{
    qDeleteAll(m_children);
}

QString AbstractInstrumentTreeItem::title() const
{
    return m_title;
}

int AbstractInstrumentTreeItem::type() const
{
    return static_cast<int>(m_type);
}

bool AbstractInstrumentTreeItem::isVisible() const
{
    return m_isVisible;
}

bool AbstractInstrumentTreeItem::isSelectable() const
{
    return m_type != InstrumentTreeItemType::ItemType::CONTROL_ADD_DOUBLE_INSTRUMENT
           || m_type != InstrumentTreeItemType::ItemType::CONTROL_ADD_STAFF;
}

AbstractInstrumentTreeItem* AbstractInstrumentTreeItem::parentItem() const
{
    return m_parent;
}

void AbstractInstrumentTreeItem::setParentItem(AbstractInstrumentTreeItem* parent)
{
    m_parent = parent;
}

QList<AbstractInstrumentTreeItem*> AbstractInstrumentTreeItem::childrenItems() const
{
    return m_children;
}

AbstractInstrumentTreeItem* AbstractInstrumentTreeItem::childAtRow(const int row)
{
    if (row < 0 || row >= childCount()) {
        return nullptr;
    }

    return static_cast<AbstractInstrumentTreeItem*>(m_children.at(row));
}

void AbstractInstrumentTreeItem::appendChild(AbstractInstrumentTreeItem* child)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.append(child);
}

void AbstractInstrumentTreeItem::insertChild(AbstractInstrumentTreeItem* child, const int beforeRow)
{
    if (!child) {
        return;
    }

    child->setParentItem(this);

    m_children.insert(beforeRow, child);
}

void AbstractInstrumentTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    if (count == 1) {
        m_children.removeAt(row);
        return;
    }

    for (int i = row + count - 1; i <= row; --i) {
        AbstractInstrumentTreeItem* child = m_children.at(i);
        m_children.removeAt(i);

        if (deleteChild) {
            child->deleteLater();
        }
    }
}

int AbstractInstrumentTreeItem::childCount() const
{
    return m_children.size();
}

int AbstractInstrumentTreeItem::row() const
{
    if (!parentItem()) {
        return 0;
    }

    return parentItem()->childrenItems().indexOf(const_cast<AbstractInstrumentTreeItem*>(this));
}

void AbstractInstrumentTreeItem::setType(InstrumentTreeItemType::ItemType type)
{
    if (m_type == type)
        return;

    m_type = type;
    emit typeChanged(m_type);
    emit isSelectableChanged(isSelectable());
}

void AbstractInstrumentTreeItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged(m_title);
}

void AbstractInstrumentTreeItem::setIsVisible(bool isVisible)
{
    if (m_isVisible == isVisible) {
        return;
    }

    m_isVisible = isVisible;

    for (int i = 0; i < childCount(); ++i) {
        childAtRow(i)->setIsVisible(isVisible);
    }

    emit isVisibleChanged(m_isVisible);
}
