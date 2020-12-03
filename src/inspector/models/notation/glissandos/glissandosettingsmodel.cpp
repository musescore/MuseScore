#include "glissandosettingsmodel.h"

GlissandoSettingsModel::GlissandoSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_GLISSANDO);
    setTitle(tr("Glissando"));
    createProperties();
}

void GlissandoSettingsModel::createProperties()
{
    m_lineType = buildPropertyItem(Ms::Pid::GLISS_TYPE);
}

void GlissandoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::GLISSANDO);
}

void GlissandoSettingsModel::loadProperties()
{
    loadPropertyItem(m_lineType);
}

void GlissandoSettingsModel::resetProperties()
{
    m_lineType->resetToDefault();
}

PropertyItem* GlissandoSettingsModel::lineType() const
{
    return m_lineType;
}
