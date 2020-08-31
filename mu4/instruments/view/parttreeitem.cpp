#include "parttreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;
using ItemType = InstrumentTreeItemType::ItemType;

PartTreeItem::PartTreeItem(INotationParts* notationParts, QObject* parent)
    : AbstractInstrumentPanelTreeItem(ItemType::PART, notationParts, parent)
{
}

void PartTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                                const int destinationRow)
{
    std::vector<QString> instrumentIdVector;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        instrumentIdVector.push_back(childAtRow(i)->id());
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::After;
    }

    AbstractInstrumentPanelTreeItem* destinationInstrumentItem = destinationParent->childAtRow(destinationRowLast);
    notationParts()->moveInstruments(instrumentIdVector, id(), destinationParent->id(), destinationInstrumentItem->id(), moveMode);

    AbstractInstrumentPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void PartTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    std::vector<QString> instrumentIdVector;

    for (int i = row; i < row + count; ++i) {
        instrumentIdVector.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        notationParts()->removeInstruments(id(), instrumentIdVector);
    }

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}
