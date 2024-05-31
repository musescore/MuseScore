#include "dynamicpopupmodel.h"

#include "log.h"

using namespace mu::notation;

DynamicPopupModel::DynamicPopupModel(QObject *parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_DYNAMIC, parent)
{
}

void DynamicPopupModel::init()
{
    AbstractElementPopupModel::init();
}
