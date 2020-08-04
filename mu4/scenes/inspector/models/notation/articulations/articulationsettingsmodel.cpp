#include "articulationsettingsmodel.h"

#include "log.h"
#include "articulation.h"

ArticulationSettingsModel::ArticulationSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_ARTICULATION);
    setTitle(tr("Articulation"));
    createProperties();
}

void ArticulationSettingsModel::openChannelAndMidiProperties()
{
    adapter()->showArticulationPropertiesDialog();
}

void ArticulationSettingsModel::createProperties()
{
    m_direction = buildPropertyItem(Ms::Pid::DIRECTION);
    m_placement = buildPropertyItem(Ms::Pid::ARTICULATION_ANCHOR);
}

void ArticulationSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::ARTICULATION, [](
                                                         const Ms::Element* element) -> bool {
        IF_ASSERT_FAILED(element) {
            return false;
        }

        const Ms::Articulation* articulation = Ms::toArticulation(element);
        IF_ASSERT_FAILED(articulation) {
            return false;
        }

        return !articulation->isOrnament();
    });
}

void ArticulationSettingsModel::loadProperties()
{
    loadPropertyItem(m_direction);
    loadPropertyItem(m_placement);
}

void ArticulationSettingsModel::resetProperties()
{
    m_direction->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* ArticulationSettingsModel::direction() const
{
    return m_direction;
}

PropertyItem* ArticulationSettingsModel::placement() const
{
    return m_placement;
}
