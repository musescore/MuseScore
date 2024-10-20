#include "restspagemodel.h"

using namespace mu::notation;

RestsPageModel::RestsPageModel(QObject* parent)
    : AbstractStyleDialogModel(parent, {
    StyleId::multiVoiceRestTwoSpaceOffset,
    StyleId::mergeMatchingRests,
    StyleId::showLedgerLinesOnBreveRests
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

StyleItem* RestsPageModel::showLedgerLinesOnBreveRests() const
{
    return styleItem(StyleId::showLedgerLinesOnBreveRests);
}
