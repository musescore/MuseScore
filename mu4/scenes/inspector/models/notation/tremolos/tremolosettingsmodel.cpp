#include "tremolosettingsmodel.h"

#include <QList>

TremoloSettingsModel::TremoloSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_TREMOLO);
    setTitle(tr("Tremolos"));
    createProperties();
}

void TremoloSettingsModel::createProperties()
{
    m_strokeStyle = buildPropertyItem(Ms::Pid::TREMOLO_STROKE_STYLE);
}

void TremoloSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::TREMOLO);
}

void TremoloSettingsModel::loadProperties()
{
    loadPropertyItem(m_strokeStyle);
}

void TremoloSettingsModel::resetProperties()
{
    m_strokeStyle->resetToDefault();
}

PropertyItem* TremoloSettingsModel::strokeStyle() const
{
    return m_strokeStyle;
}
