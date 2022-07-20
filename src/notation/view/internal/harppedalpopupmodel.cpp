#include "harppedalpopupmodel.h"
#include "log.h"

using namespace mu::notation;

HarpPedalPopupModel::HarpPedalPopupModel(QObject* parent)
    : AbstractElementPopupModel(parent)
{
    setModelType(PopupModelType::TYPE_HARP_DIAGRAM);
    setTitle("Harp pedal");

    LOGD() << "Created harppedalpopup";
}
