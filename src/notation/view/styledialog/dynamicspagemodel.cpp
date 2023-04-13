#include "dynamicspagemodel.h"

using namespace mu::notation;

DynamicsPageModel::DynamicsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::centerOnNotehead,
})
{
}

StyleItem* DynamicsPageModel::useTextAlignment() const
{
    return styleItem(StyleId::centerOnNotehead);
}
