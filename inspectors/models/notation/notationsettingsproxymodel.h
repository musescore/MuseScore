#ifndef NOTATIONSETTINGSPROXYMODEL_H
#define NOTATIONSETTINGSPROXYMODEL_H

#include "models/abstractinspectormodel.h"

#include "notes/notesettingsproxymodel.h"
#include "fermatas/fermatasettingsmodel.h"
#include "tempos/temposettingsmodel.h"
#include "glissandos/glissandosettingsmodel.h"
#include "barlines/barlinesettingsmodel.h"
#include "staffs/staffsettingsmodel.h"
#include "sectionbreaks/sectionbreaksettingsmodel.h"

class NotationSettingsProxyModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(QObject* noteSettingsModel READ noteSettingsModel CONSTANT)
    Q_PROPERTY(QObject* fermataSettingsModel READ fermataSettingsModel CONSTANT)
    Q_PROPERTY(QObject* tempoSettingsModel READ tempoSettingsModel CONSTANT)
    Q_PROPERTY(QObject* glissandoSettingsModel READ glissandoSettingsModel CONSTANT)
    Q_PROPERTY(QObject* barlineSettingsModel READ barlineSettingsModel CONSTANT)
    Q_PROPERTY(QObject* staffSettingsModel READ staffSettingsModel CONSTANT)
    Q_PROPERTY(QObject* sectionBreakSettingsModel READ sectionBreakSettingsModel CONSTANT)

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
    QObject* staffSettingsModel() const;
    QObject* sectionBreakSettingsModel() const;

public slots:
    void setNoteSettingsModel(NoteSettingsProxyModel* noteSettingsModel);
    void setFermataSettingsModel(FermataSettingsModel* fermataSettingsModel);
    void setTempoSettingsModel(TempoSettingsModel* tempoSettingsModel);
    void setGlissandoSettingsModel(GlissandoSettingsModel* glissandoSettingsModel);
    void setBarlineSettingsModel(BarlineSettingsModel* barlineSettingsModel);
    void setStaffSettingsModel(StaffSettingsModel* staffSettingsModel);
    void setSectionBreakSettingsModel(SectionBreakSettingsModel* sectionBreakSettingsModel);

private:
    NoteSettingsProxyModel* m_noteSettingsModel = nullptr;
    FermataSettingsModel* m_fermataSettingsModel = nullptr;
    TempoSettingsModel* m_tempoSettingsModel = nullptr;
    GlissandoSettingsModel* m_glissandoSettingsModel = nullptr;
    BarlineSettingsModel* m_barlineSettingsModel = nullptr;
    StaffSettingsModel* m_staffSettingsModel = nullptr;
    SectionBreakSettingsModel* m_sectionBreakSettingsModel = nullptr;
};

#endif // NOTATIONSETTINGSPROXYMODEL_H
