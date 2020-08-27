#include "ornamentsettingsmodel.h"

#include "log.h"
#include "articulation.h"

OrnamentSettingsModel::OrnamentSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_ORNAMENT);
    setTitle(tr("Ornament"));
    createProperties();
}

void OrnamentSettingsModel::openChannelAndMidiProperties()
{
    adapter()->showArticulationPropertiesDialog();
}

void OrnamentSettingsModel::createProperties()
{
    m_performanceType = buildPropertyItem(Ms::Pid::ORNAMENT_STYLE);
    m_placement = buildPropertyItem(Ms::Pid::ARTICULATION_ANCHOR);
}

void OrnamentSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::ARTICULATION, [](const Ms::Element* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Articulation* articulation = Ms::toArticulation(element);
        IF_ASSERT_FAILED(articulation) {
            return false;
        }

        return articulation->isOrnament();
    });
}

void OrnamentSettingsModel::loadProperties()
{
    loadPropertyItem(m_performanceType);
    loadPropertyItem(m_placement);
}

void OrnamentSettingsModel::resetProperties()
{
    m_performanceType->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* OrnamentSettingsModel::performanceType() const
{
    return m_performanceType;
}

PropertyItem* OrnamentSettingsModel::placement() const
{
    return m_placement;
}
