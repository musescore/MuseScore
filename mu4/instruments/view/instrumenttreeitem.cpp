#include "instrumenttreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;

InstrumentTreeItem::InstrumentTreeItem(INotationParts* notationParts, QObject* parent) :
    AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::INSTRUMENT, notationParts, parent)
{
}

void InstrumentTreeItem::moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent,
                                      const int destinationRow)
{
    std::vector<int> stafftIdVector;

    for (int i = sourceRow; i < sourceRow + count; ++i) {
        stafftIdVector.push_back(childAtRow(i)->id().toInt());
    }

    int destinationRowLast = destinationRow;
    INotationParts::InsertMode moveMode = INotationParts::Before;

    // exclude the control item
    if (destinationRow == destinationParent->childCount() - 1) {
        destinationRowLast = destinationRow - 1;
        moveMode = INotationParts::After;
    }

    AbstractInstrumentPanelTreeItem* destinationInstrumentItem = destinationParent->childAtRow(destinationRowLast);
    notationParts()->moveStaves(stafftIdVector, destinationInstrumentItem->id().toInt(), moveMode);

    AbstractInstrumentPanelTreeItem::moveChildren(sourceRow, count, destinationParent, destinationRow);
}

void InstrumentTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    std::vector<int> staffIdVector;

    for (int i = row; i < row + count; ++i) {
        staffIdVector.push_back(childAtRow(i)->id().toInt());
    }

    if (deleteChild) {
        notationParts()->removeStaves(staffIdVector);
    }

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}
