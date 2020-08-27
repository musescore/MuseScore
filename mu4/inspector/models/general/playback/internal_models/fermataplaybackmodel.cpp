#include "fermataplaybackmodel.h"

#include "dataformatter.h"

FermataPlaybackModel::FermataPlaybackModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(tr("Fermatas"));

    createProperties();
}

void FermataPlaybackModel::createProperties()
{
    m_timeStretch = buildPropertyItem(Ms::Pid::TIME_STRETCH, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue.toDouble() / 100);
    });
}

void FermataPlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::FERMATA);
}

void FermataPlaybackModel::loadProperties()
{
    loadPropertyItem(m_timeStretch, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble()) * 100;
    });
}

void FermataPlaybackModel::resetProperties()
{
    m_timeStretch->resetToDefault();
}

PropertyItem* FermataPlaybackModel::timeStretch() const
{
    return m_timeStretch;
}
