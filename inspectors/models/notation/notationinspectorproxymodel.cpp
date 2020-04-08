#include "notationinspectorproxymodel.h"

NotationInspectorProxyModel::NotationInspectorProxyModel(QObject* parent, IElementRepositoryService* repository) : AbstractInspectorModel(parent)
{
    setTitle(QStringLiteral("Notation"));
    setType(NOTATION);
    setStemSettingsModel(new StemSettingsModel(this, repository));
    setHeadSettingsModel(new NoteheadSettingsModel(this, repository));
    setBeamSettingsModel(new BeamSettingsModel(this, repository));
    setHookSettingsModel(new HookSettingsModel(this, repository));
}

void NotationInspectorProxyModel::requestResetToDefaults()
{
    m_stemSettingsModel->requestResetToDefaults();
    m_headSettingsModel->requestResetToDefaults();
    m_beamSettingsModel->requestResetToDefaults();
    m_hookSettingsModel->requestResetToDefaults();
}

bool NotationInspectorProxyModel::hasAcceptableElements() const
{
    return m_stemSettingsModel->hasAcceptableElements() ||
           m_headSettingsModel->hasAcceptableElements() ||
           m_beamSettingsModel->hasAcceptableElements() ||
           m_hookSettingsModel->hasAcceptableElements();
}

QObject* NotationInspectorProxyModel::beamSettingsModel() const
{
    return m_beamSettingsModel;
}

QObject* NotationInspectorProxyModel::headSettingsModel() const
{
    return m_headSettingsModel;
}

QObject* NotationInspectorProxyModel::stemSettingsModel() const
{
    return m_stemSettingsModel;
}

QObject* NotationInspectorProxyModel::hookSettingsModel() const
{
    return m_hookSettingsModel;
}

void NotationInspectorProxyModel::setBeamSettingsModel(BeamSettingsModel* beamSettingsModel)
{
    m_beamSettingsModel = beamSettingsModel;
    emit beamSettingsModelChanged(m_beamSettingsModel);
}

void NotationInspectorProxyModel::setHeadSettingsModel(NoteheadSettingsModel* headSettingsModel)
{
    m_headSettingsModel = headSettingsModel;
    emit headSettingsModelChanged(m_headSettingsModel);
}

void NotationInspectorProxyModel::setStemSettingsModel(StemSettingsModel* stemSettingsModel)
{
    m_stemSettingsModel = stemSettingsModel;
    emit stemSettingsModelChanged(m_stemSettingsModel);
}

void NotationInspectorProxyModel::setHookSettingsModel(HookSettingsModel* hookSettingsModel)
{
    m_hookSettingsModel = hookSettingsModel;
    emit hookSettingsModelChanged(m_hookSettingsModel);
}
