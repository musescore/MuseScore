#ifndef NOTEPLAYBACKMODEL_H
#define NOTEPLAYBACKMODEL_H

#include "models/abstractinspectormodel.h"

class NotePlaybackModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * tuning READ tuning CONSTANT)
    Q_PROPERTY(PropertyItem * velocity READ velocity CONSTANT)
    Q_PROPERTY(PropertyItem * overrideDynamics READ overrideDynamics CONSTANT)

public:
    explicit NotePlaybackModel(QObject* parent, IElementRepositoryService* repository);

    PropertyItem* tuning() const;
    PropertyItem* velocity() const;
    PropertyItem* overrideDynamics() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

private:
    PropertyItem* m_tuning = nullptr;
    PropertyItem* m_velocity = nullptr;
    PropertyItem* m_overrideDynamics = nullptr;
};

#endif // NOTEPLAYBACKMODEL_H
