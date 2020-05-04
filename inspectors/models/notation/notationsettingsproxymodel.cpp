#include "notationsettingsproxymodel.h"

NotationSettingsProxyModel::NotationSettingsProxyModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setSectionType(SECTION_NOTATION);
    setTitle(QStringLiteral("Notation"));

    setNoteSettingsModel(new NoteSettingsProxyModel(this, repository));
    setFermataSettingsModel(new FermataSettingsModel(this, repository));
    setTempoSettingsModel(new TempoSettingsModel(this, repository));
    setGlissandoSettingsModel(new GlissandoSettingsModel(this, repository));
    setBarlineSettingsModel(new BarlineSettingsModel(this, repository));
    setStaffSettingsModel(new StaffSettingsModel(this, repository));
    setSectionBreakSettingsModel(new SectionBreakSettingsModel(this, repository));
    setMarkerSettingsModel(new MarkerSettingsModel(this, repository));
    setJumpSettingsModel(new JumpSettingsModel(this, repository));
}

bool NotationSettingsProxyModel::hasAcceptableElements() const
{
    return m_noteSettingsModel->hasAcceptableElements();
}

QObject* NotationSettingsProxyModel::noteSettingsModel() const
{
    return m_noteSettingsModel;
}

QObject* NotationSettingsProxyModel::fermataSettingsModel() const
{
    return m_fermataSettingsModel;
}

QObject* NotationSettingsProxyModel::tempoSettingsModel() const
{
    return m_tempoSettingsModel;
}

QObject* NotationSettingsProxyModel::glissandoSettingsModel() const
{
    return m_glissandoSettingsModel;
}

QObject* NotationSettingsProxyModel::barlineSettingsModel() const
{
    return m_barlineSettingsModel;
}

QObject* NotationSettingsProxyModel::staffSettingsModel() const
{
    return m_staffSettingsModel;
}

QObject* NotationSettingsProxyModel::sectionBreakSettingsModel() const
{
    return m_sectionBreakSettingsModel;
}

QObject* NotationSettingsProxyModel::markerSettingsModel() const
{
    return m_markerSettingsModel;
}

QObject* NotationSettingsProxyModel::jumpSettingsModel() const
{
    return m_jumpSettingsModel;
}

void NotationSettingsProxyModel::setNoteSettingsModel(NoteSettingsProxyModel* noteSettingsModel)
{
    m_noteSettingsModel = noteSettingsModel;
}

void NotationSettingsProxyModel::setFermataSettingsModel(FermataSettingsModel* fermataSettingsModel)
{
    m_fermataSettingsModel = fermataSettingsModel;
}

void NotationSettingsProxyModel::setTempoSettingsModel(TempoSettingsModel* tempoSettingsModel)
{
    m_tempoSettingsModel = tempoSettingsModel;
}

void NotationSettingsProxyModel::setGlissandoSettingsModel(GlissandoSettingsModel* glissandoSettingsModel)
{
    m_glissandoSettingsModel = glissandoSettingsModel;
}

void NotationSettingsProxyModel::setBarlineSettingsModel(BarlineSettingsModel* barlineSettingsModel)
{
    m_barlineSettingsModel = barlineSettingsModel;
}

void NotationSettingsProxyModel::setStaffSettingsModel(StaffSettingsModel* staffSettingsModel)
{
    m_staffSettingsModel = staffSettingsModel;
}

void NotationSettingsProxyModel::setSectionBreakSettingsModel(SectionBreakSettingsModel* sectionBreakSettingsModel)
{
    m_sectionBreakSettingsModel = sectionBreakSettingsModel;
}

void NotationSettingsProxyModel::setMarkerSettingsModel(MarkerSettingsModel* markerSettingsModel)
{
    m_markerSettingsModel = markerSettingsModel;
}

void NotationSettingsProxyModel::setJumpSettingsModel(JumpSettingsModel* jumpSettingsModel)
{
    m_jumpSettingsModel = jumpSettingsModel;
}
