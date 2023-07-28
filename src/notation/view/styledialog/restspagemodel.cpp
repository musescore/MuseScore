#include "restspagemodel.h"

using namespace mu::notation;

RestsPageModel::RestsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::multiVoiceRestTwoSpaceOffset
})
{
}

StyleItem* RestsPageModel::multiVoiceRestTwoSpaceOffset() const
{
    return styleItem(StyleId::multiVoiceRestTwoSpaceOffset);
}
