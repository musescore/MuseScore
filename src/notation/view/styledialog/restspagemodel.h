#ifndef RESTSPAGEMODEL_H
#define RESTSPAGEMODEL_H

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class RestsPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * multiVoiceRestTwoSpaceOffset READ multiVoiceRestTwoSpaceOffset CONSTANT)
    Q_PROPERTY(StyleItem * staffDefaultMergeMatchingRests READ staffDefaultMergeMatchingRests CONSTANT)

public:
    explicit RestsPageModel(QObject* parent = nullptr);

    StyleItem* multiVoiceRestTwoSpaceOffset() const;
    StyleItem* staffDefaultMergeMatchingRests() const;
};
}

#endif // RESTSPAGEMODEL_H
