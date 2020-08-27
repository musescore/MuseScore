#ifndef ARPEGGIOPLAYBACKMODEL_H
#define ARPEGGIOPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

class ArpeggioPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * stretch READ stretch CONSTANT)
public:
    explicit ArpeggioPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* stretch() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_stretch = nullptr;
};

#endif // ARPEGGIOPLAYBACKMODEL_H
