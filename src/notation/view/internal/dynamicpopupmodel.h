#ifndef MU_NOTATION_DYNAMICPOPUPMODEL_H
#define MU_NOTATION_DYNAMICPOPUPMODEL_H

#include <view/abstractelementpopupmodel.h>

namespace mu::notation {
class DynamicPopupModel : public AbstractElementPopupModel
{
public:
    explicit DynamicPopupModel(QObject *parent = nullptr);

    Q_INVOKABLE void init() override;
};
} // namespace mu::notation

#endif // MU_NOTATION_DYNAMICPOPUPMODEL_H
