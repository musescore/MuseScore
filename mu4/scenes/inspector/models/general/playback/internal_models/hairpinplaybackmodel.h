#ifndef HAIRPINPLAYBACKMODEL_H
#define HAIRPINPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

class HairpinPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * velocityChange READ velocityChange CONSTANT)
    Q_PROPERTY(PropertyItem * velocityChangeType READ velocityChangeType CONSTANT)

public:
    explicit HairpinPlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* velocityChange() const;
    PropertyItem* velocityChangeType() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_velocityChange = nullptr;
    PropertyItem* m_velocityChangeType = nullptr;
};

#endif // HAIRPINPLAYBACKMODEL_H
