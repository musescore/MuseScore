#include "keysignaturesettingsmodel.h"

KeySignatureSettingsModel::KeySignatureSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_KEYSIGNATURE);
    setTitle(tr("Key signature"));
    createProperties();
}

void KeySignatureSettingsModel::createProperties()
{
    m_hasToShowCourtesy = buildPropertyItem(Ms::Pid::SHOW_COURTESY);
    m_mode = buildPropertyItem(Ms::Pid::KEYSIG_MODE);
}

void KeySignatureSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::KEYSIG);
}

void KeySignatureSettingsModel::loadProperties()
{
    loadPropertyItem(m_hasToShowCourtesy, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toBool();
    });

    loadPropertyItem(m_mode);
}

void KeySignatureSettingsModel::resetProperties()
{
    m_hasToShowCourtesy->resetToDefault();
    m_mode->resetToDefault();
}

PropertyItem* KeySignatureSettingsModel::hasToShowCourtesy() const
{
    return m_hasToShowCourtesy;
}

PropertyItem* KeySignatureSettingsModel::mode() const
{
    return m_mode;
}
