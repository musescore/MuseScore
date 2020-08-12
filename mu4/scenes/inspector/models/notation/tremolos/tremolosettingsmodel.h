#ifndef TREMOLOSETTINGSMODEL_H
#define TREMOLOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

class TremoloSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * strokeStyle READ strokeStyle CONSTANT)

public:
    explicit TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* strokeStyle() const;

private:
    PropertyItem* m_strokeStyle = nullptr;
};

#endif // TREMOLOSETTINGSMODEL_H
