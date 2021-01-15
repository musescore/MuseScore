#include "tremolosettingsmodel.h"

#include <QList>

using namespace mu::inspector;

TremoloSettingsModel::TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_TREMOLO);
    setTitle(qtrc("inspector", "Tremolos"));
    createProperties();
}

void TremoloSettingsModel::createProperties()
{
    m_style = buildPropertyItem(Ms::Pid::TREMOLO_STYLE);
}

void TremoloSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TREMOLO);
}

void TremoloSettingsModel::loadProperties()
{
    loadPropertyItem(m_style);
}

void TremoloSettingsModel::resetProperties()
{
    m_style->resetToDefault();
}

PropertyItem* TremoloSettingsModel::style() const
{
    return m_style;
}
