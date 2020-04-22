#ifndef NOTATIONSETTINGSPROXYMODEL_H
#define NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectormodel.h"

#include "notes/notesettingsproxymodel.h"
#include "fermatas/fermatasettingsmodel.h"
#include "tempos/temposettingsmodel.h"
#include "glissandos/glissandosettingsmodel.h"

class NotationSettingsProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteSettingsModel READ noteSettingsModel CONSTANT)
    Q_PROPERTY(QObject* fermataSettingsModel READ fermataSettingsModel CONSTANT)
    Q_PROPERTY(QObject* tempoSettingsModel READ tempoSettingsModel CONSTANT)
    Q_PROPERTY(QObject* glissandoSettingsModel READ glissandoSettingsModel CONSTANT)
public:
    explicit NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override {}
    void requestElements() override {}
    void loadProperties() override {}
    void resetProperties() override {}

    bool hasAcceptableElements() const override;

    QObject* noteSettingsModel() const;
    QObject* fermataSettingsModel() const;
    QObject* tempoSettingsModel() const;
    QObject* glissandoSettingsModel() const;

public slots:
    void setNoteSettingsModel(NoteSettingsProxyModel* noteSettingsModel);
    void setFermataSettingsModel(FermataSettingsModel* fermataSettingsModel);
    void setTempoSettingsModel(TempoSettingsModel* tempoSettingsModel);
    void setGlissandoSettingsModel(GlissandoSettingsModel* glissandoSettingsModel);

private:
    NoteSettingsProxyModel* m_noteSettingsModel = nullptr;
    FermataSettingsModel* m_fermataSettingsModel = nullptr;
    QObject* m_tempoSettingsModel = nullptr;
    QObject* m_glissandoSettingsModel = nullptr;
};

#endif // NOTATIONSETTINGSPROXYMODEL_H
