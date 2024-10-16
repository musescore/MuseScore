#include "restspagemodel.h"

using namespace mu::notation;

RestsPageModel::RestsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::multiVoiceRestTwoSpaceOffset,
    StyleId::mergeMatchingRests
})
{
}

StyleItem* RestsPageModel::multiVoiceRestTwoSpaceOffset() const
{
    return styleItem(StyleId::multiVoiceRestTwoSpaceOffset);
}

StyleItem* RestsPageModel::mergeMatchingRests() const
{
    return styleItem(StyleId::mergeMatchingRests);
}
