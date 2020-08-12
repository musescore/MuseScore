#ifndef BEAMMODESMODEL_H
#define BEAMMODESMODEL_H

#include "beammodelistmodel.h"
#include "models/abstractinspectormodel.h"

class BeamModesModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject * modeListModel READ modeListModel NOTIFY modeListModelChanged)
    Q_PROPERTY(PropertyItem * mode READ mode CONSTANT)
    Q_PROPERTY(PropertyItem * isFeatheringAvailable READ isFeatheringAvailable CONSTANT)

public:
    explicit BeamModesModel(QObject* parent, IElementRepositoryService* repository);

    QObject* modeListModel() const;
    PropertyItem* mode() const;
    PropertyItem* isFeatheringAvailable() const;

public slots:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    void setModeListModel(BeamModeListModel* modeListModel);

signals:
    void modeListModelChanged(QObject* modeListModel);

private:
    BeamModeListModel* m_modeListModel = nullptr;
    PropertyItem* m_mode = nullptr;
    PropertyItem* m_isFeatheringAvailable = nullptr;
};

#endif // BEAMMODESMODEL_H
