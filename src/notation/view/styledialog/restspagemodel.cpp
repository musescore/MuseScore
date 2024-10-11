#include "restspagemodel.h"

using namespace mu::notation;

RestsPageModel::RestsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::multiVoiceRestTwoSpaceOffset,
    StyleId::staffDefaultMergeMatchingRests
})
{
}

StyleItem* RestsPageModel::multiVoiceRestTwoSpaceOffset() const
{
    return styleItem(StyleId::multiVoiceRestTwoSpaceOffset);
}

StyleItem* RestsPageModel::staffDefaultMergeMatchingRests() const
{
    return styleItem(StyleId::staffDefaultMergeMatchingRests);
}
