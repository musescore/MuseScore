#ifndef NOTATIONSETTINGSPROXYMODEL_H
#define NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectormodel.h"

#include "notes/notesettingsproxymodel.h"
#include "fermatas/fermatasettingsmodel.h"
#include "tempos/temposettingsmodel.h"
#include "glissandos/glissandosettingsmodel.h"
#include "barlines/barlinesettingsmodel.h"

class NotationSettingsProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteSettingsModel READ noteSettingsModel CONSTANT)
    Q_PROPERTY(QObject* fermataSettingsModel READ fermataSettingsModel CONSTANT)
    Q_PROPERTY(QObject* tempoSettingsModel READ tempoSettingsModel CONSTANT)
    Q_PROPERTY(QObject* glissandoSettingsModel READ glissandoSettingsModel CONSTANT)
    Q_PROPERTY(QObject* barlineSettingsModel READ barlineSettingsModel CONSTANT)
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
    QObject* barlineSettingsModel() const;

public slots:
    void setNoteSettingsModel(NoteSettingsProxyModel* noteSettingsModel);
    void setFermataSettingsModel(FermataSettingsModel* fermataSettingsModel);
    void setTempoSettingsModel(TempoSettingsModel* tempoSettingsModel);
    void setGlissandoSettingsModel(GlissandoSettingsModel* glissandoSettingsModel);
    void setBarlineSettingsModel(BarlineSettingsModel* barlineSettingsModel);

private:
    NoteSettingsProxyModel* m_noteSettingsModel = nullptr;
    FermataSettingsModel* m_fermataSettingsModel = nullptr;
    TempoSettingsModel* m_tempoSettingsModel = nullptr;
    GlissandoSettingsModel* m_glissandoSettingsModel = nullptr;
    BarlineSettingsModel* m_barlineSettingsModel = nullptr;
};

#endif // NOTATIONSETTINGSPROXYMODEL_H
