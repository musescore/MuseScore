#ifndef BREATHPLAYBACKMODEL_H
#define BREATHPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

class BreathPlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * pauseTime READ pauseTime CONSTANT)

public:
    explicit BreathPlaybackModel(QObject* parent, IElementRepositoryService* repository);

public:
    PropertyItem* pauseTime() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_pauseTime = nullptr;
};

#endif // BREATHPLAYBACKMODEL_H
