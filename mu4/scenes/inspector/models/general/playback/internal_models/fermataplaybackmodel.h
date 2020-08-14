#ifndef FERMATAPLAYBACKMODEL_H
#define FERMATAPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

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

#endif // FERMATAPLAYBACKMODEL_H
