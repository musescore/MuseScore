#ifndef DYNAMICSPAGEMODEL_H
#define DYNAMICSPAGEMODEL_H

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class DynamicsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * useTextAlignment READ useTextAlignment CONSTANT)

public:
    explicit DynamicsPageModel(QObject* parent = nullptr);

    StyleItem* useTextAlignment() const;
};
}

#endif // DYNAMICSPAGEMODEL_H
