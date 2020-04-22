#include "notesettingsproxymodel.h"

NoteSettingsProxyModel::NoteSettingsProxyModel(QObject* parent, IElementRepositoryService* repository) : AbstractInspectorModel(parent)
{
    setSectionType(SECTION_NOTATION);
    setModelType(TYPE_NOTE);
    setStemSettingsModel(new StemSettingsModel(this, repository));
    setHeadSettingsModel(new NoteheadSettingsModel(this, repository));
    setBeamSettingsModel(new BeamSettingsModel(this, repository));
    setHookSettingsModel(new HookSettingsModel(this, repository));
}

void NoteSettingsProxyModel::requestResetToDefaults()
{
    m_stemSettingsModel->requestResetToDefaults();
    m_headSettingsModel->requestResetToDefaults();
    m_beamSettingsModel->requestResetToDefaults();
    m_hookSettingsModel->requestResetToDefaults();
}

bool NoteSettingsProxyModel::hasAcceptableElements() const
{
    return m_stemSettingsModel->hasAcceptableElements() ||
           m_headSettingsModel->hasAcceptableElements() ||
           m_beamSettingsModel->hasAcceptableElements() ||
           m_hookSettingsModel->hasAcceptableElements();
}

QObject* NoteSettingsProxyModel::beamSettingsModel() const
{
    return m_beamSettingsModel;
}

QObject* NoteSettingsProxyModel::headSettingsModel() const
{
    return m_headSettingsModel;
}

QObject* NoteSettingsProxyModel::stemSettingsModel() const
{
    return m_stemSettingsModel;
}

QObject* NoteSettingsProxyModel::hookSettingsModel() const
{
    return m_hookSettingsModel;
}

void NoteSettingsProxyModel::setBeamSettingsModel(BeamSettingsModel* beamSettingsModel)
{
    m_beamSettingsModel = beamSettingsModel;
    emit beamSettingsModelChanged(m_beamSettingsModel);
}

void NoteSettingsProxyModel::setHeadSettingsModel(NoteheadSettingsModel* headSettingsModel)
{
    m_headSettingsModel = headSettingsModel;
    emit headSettingsModelChanged(m_headSettingsModel);
}

void NoteSettingsProxyModel::setStemSettingsModel(StemSettingsModel* stemSettingsModel)
{
    m_stemSettingsModel = stemSettingsModel;
    emit stemSettingsModelChanged(m_stemSettingsModel);
}

void NoteSettingsProxyModel::setHookSettingsModel(HookSettingsModel* hookSettingsModel)
{
    m_hookSettingsModel = hookSettingsModel;
    emit hookSettingsModelChanged(m_hookSettingsModel);
}
