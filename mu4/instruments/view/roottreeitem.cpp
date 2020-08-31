#include "roottreeitem.h"

using namespace mu::instruments;
using namespace mu::notation;

RootTreeItem::RootTreeItem(INotationParts* notationParts, QObject* parent) :
    AbstractInstrumentPanelTreeItem(InstrumentTreeItemType::ItemType::ROOT, notationParts, parent)
{
}

void RootTreeItem::removeChildren(const int row, const int count, const bool deleteChild)
{
    std::vector<QString> partIdVector;

    for (int i = row; i < row + count; ++i) {
        partIdVector.push_back(childAtRow(i)->id());
    }

    if (deleteChild) {
        notationParts()->removeParts(partIdVector);
    }

    AbstractInstrumentPanelTreeItem::removeChildren(row, count, deleteChild);
}
