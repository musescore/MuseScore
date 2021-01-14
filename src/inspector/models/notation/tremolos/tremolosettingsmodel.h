#ifndef TREMOLOSETTINGSMODEL_H
#define TREMOLOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TremoloSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * style READ style CONSTANT)

public:
    explicit TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* style() const;

private:
    PropertyItem* m_style = nullptr;
};
}

#endif // TREMOLOSETTINGSMODEL_H
