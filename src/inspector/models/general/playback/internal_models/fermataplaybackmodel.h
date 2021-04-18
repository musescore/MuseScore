#ifndef MU_INSPECTOR_FERMATAPLAYBACKMODEL_H
#define MU_INSPECTOR_FERMATAPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class FermataPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * timeStretch READ timeStretch CONSTANT)

public:
    explicit FermataPlaybackModel(QObject* parent, IElementRepositoryService* repository);

public:
    PropertyItem* timeStretch() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_timeStretch = nullptr;
};
}

#endif // MU_INSPECTOR_FERMATAPLAYBACKMODEL_H
