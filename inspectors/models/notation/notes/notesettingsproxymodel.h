#ifndef NOTATIONINSPECTORPROXYMODEL_H
#define NOTATIONINSPECTORPROXYMODEL_H

#include "models/abstractinspectormodel.h"
#include "stems/stemsettingsmodel.h"
#include "noteheads/noteheadsettingsmodel.h"
#include "beams/beamsettingsmodel.h"
#include "hooks/hooksettingsmodel.h"

class NoteSettingsProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject* beamSettingsModel READ beamSettingsModel NOTIFY beamSettingsModelChanged)
    Q_PROPERTY(QObject* headSettingsModel READ headSettingsModel NOTIFY headSettingsModelChanged)
    Q_PROPERTY(QObject* stemSettingsModel READ stemSettingsModel NOTIFY stemSettingsModelChanged)
    Q_PROPERTY(QObject* hookSettingsModel READ hookSettingsModel NOTIFY hookSettingsModelChanged)

public:
    explicit NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);

    void requestResetToDefaults() override;
    bool hasAcceptableElements() const override;

    QObject* beamSettingsModel() const;
    QObject* headSettingsModel() const;
    QObject* stemSettingsModel() const;
    QObject* hookSettingsModel() const;

public slots:
    void setBeamSettingsModel(BeamSettingsModel* beamSettingsModel);
    void setHeadSettingsModel(NoteheadSettingsModel* headSettingsModel);
    void setStemSettingsModel(StemSettingsModel* stemSettingsModel);
    void setHookSettingsModel(HookSettingsModel* hookSettingsModel);

signals:
    void beamSettingsModelChanged(QObject* beamSettingsModel);
    void headSettingsModelChanged(QObject* headSettingsModel);
    void stemSettingsModelChanged(QObject* stemSettingsModel);
    void hookSettingsModelChanged(QObject* hookSettingsModel);

private:
    void createProperties() override {}
    void requestElements() override {}
    void loadProperties() override {}
    void resetProperties() override {}

    BeamSettingsModel* m_beamSettingsModel = nullptr;
    StemSettingsModel* m_stemSettingsModel = nullptr;
    NoteheadSettingsModel* m_headSettingsModel = nullptr;
    HookSettingsModel* m_hookSettingsModel = nullptr;
};

#endif // NOTATIONINSPECTORPROXYMODEL_H
