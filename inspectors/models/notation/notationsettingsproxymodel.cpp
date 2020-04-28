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