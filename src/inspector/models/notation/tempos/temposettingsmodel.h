#ifndef MU_INSPECTOR_TEMPOSETTINGSMODEL_H
#define MU_INSPECTOR_TEMPOSETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class TempoSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * isDefaultTempoForced READ isDefaultTempoForced CONSTANT)
    Q_PROPERTY(PropertyItem * tempo READ tempo CONSTANT)

public:
    explicit TempoSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* isDefaultTempoForced() const;
    PropertyItem* tempo() const;

private:
    PropertyItem* m_isDefaultTempoForced = nullptr;
    PropertyItem* m_tempo = nullptr;
};
}

#endif // MU_INSPECTOR_TEMPOSETTINGSMODEL_H
