#ifndef ROOTTREEITEM_H
#define ROOTTREEITEM_H

#include "abstractinstrumentpaneltreeitem.h"

#include "notation/inotationparts.h"

namespace mu {
namespace instruments {

class RootTreeItem : public AbstractInstrumentPanelTreeItem
{
    Q_OBJECT
public:
    explicit RootTreeItem(notation::INotationParts* notationParts, QObject* parent = nullptr);

    void removeChildren(const int row, const int count, const bool deleteChild) override;
};
}
}

#endif // ROOTTREEITEM_H
