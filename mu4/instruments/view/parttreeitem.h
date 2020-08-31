#ifndef PARTTREEITEM_H
#define PARTTREEITEM_H

#include "abstractinstrumentpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu {
namespace instruments {
class PartTreeItem : public AbstractInstrumentPanelTreeItem
{
    Q_OBJECT
public:
    explicit PartTreeItem(notation::INotationParts* notationParts, QObject* parent = nullptr);

    void moveChildren(const int sourceRow, const int count, AbstractInstrumentPanelTreeItem* destinationParent, const int destinationRow) override;
    void removeChildren(const int row, const int count, const bool deleteChild) override;
};
}
}

#endif // PARTTREEITEM_H
