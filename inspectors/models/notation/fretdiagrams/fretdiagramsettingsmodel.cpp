#include "fretdiagramsettingsmodel.h"

#include "dataformatter.h"

FretDiagramSettingsModel::FretDiagramSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_FRET_DIAGRAM);
    setTitle(tr("Fretboard Diagram"));
    createProperties();
}

void FretDiagramSettingsModel::createProperties()
{
    m_scale = buildPropertyItem(Ms::Pid::MAG, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue.toDouble() / 100);
    });

    m_stringsCount = buildPropertyItem(Ms::Pid::FRET_STRINGS, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);
        emit fretDiagramChanged(fretDiagram());
    });

    m_fretsCount = buildPropertyItem(Ms::Pid::FRET_FRETS, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);
        emit fretDiagramChanged(fretDiagram());
    });

    m_startingFretNumber = buildPropertyItem(Ms::Pid::FRET_OFFSET, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue.toInt() - 1);
        emit fretDiagramChanged(fretDiagram());
    });

    m_isNutVisible = buildPropertyItem(Ms::Pid::FRET_NUT, [this](const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        emit fretDiagramChanged(fretDiagram());
    });

    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);
}

void FretDiagramSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::FRET_DIAGRAM);

    emit fretDiagramChanged(fretDiagram());
    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void FretDiagramSettingsModel::loadProperties()
{
    loadPropertyItem(m_scale, [](const QVariant& elementPropertyValue) -> QVariant {
        return DataFormatter::formatDouble(elementPropertyValue.toDouble()) * 100;
    });

    loadPropertyItem(m_stringsCount);
    loadPropertyItem(m_fretsCount);
    loadPropertyItem(m_startingFretNumber, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt() + 1;
    });

    loadPropertyItem(m_isNutVisible);
    loadPropertyItem(m_placement);
}

void FretDiagramSettingsModel::resetProperties()
{
    m_scale->resetToDefault();
    m_stringsCount->resetToDefault();
    m_fretsCount->resetToDefault();
    m_startingFretNumber->resetToDefault();
    m_isNutVisible->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* FretDiagramSettingsModel::scale() const
{
    return m_scale;
}

PropertyItem* FretDiagramSettingsModel::stringsCount() const
{
    return m_stringsCount;
}

PropertyItem* FretDiagramSettingsModel::fretsCount() const
{
    return m_fretsCount;
}

PropertyItem* FretDiagramSettingsModel::isNutVisible() const
{
    return m_isNutVisible;
}

PropertyItem* FretDiagramSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* FretDiagramSettingsModel::startingFretNumber() const
{
    return m_startingFretNumber;
}

QVariant FretDiagramSettingsModel::fretDiagram() const
{
    if (m_elementList.isEmpty()) {
        return QVariant();
    }

    return QVariant::fromValue(Ms::toFretDiagram(m_elementList.first()));
}

bool FretDiagramSettingsModel::isBarreModeOn() const
{
    return m_isBarreModeOn;
}

bool FretDiagramSettingsModel::isMultipleDotsModeOn() const
{
    return m_isMultipleDotsModeOn;
}

int FretDiagramSettingsModel::currentFretDotType() const
{
    return static_cast<int>(m_currentFretDotType);
}

bool FretDiagramSettingsModel::areSettingsAvailable() const
{
    return m_elementList.count() == 1; // FretDiagram inspector doesn't support multiple selection
}

void FretDiagramSettingsModel::setIsBarreModeOn(bool isBarreModeOn)
{
    if (m_isBarreModeOn == isBarreModeOn) {
        return;
    }

    m_isBarreModeOn = isBarreModeOn;
    emit isBarreModeOnChanged(m_isBarreModeOn);
}

void FretDiagramSettingsModel::setIsMultipleDotsModeOn(bool isMultipleDotsModeOn)
{
    if (m_isMultipleDotsModeOn == isMultipleDotsModeOn) {
        return;
    }

    m_isMultipleDotsModeOn = isMultipleDotsModeOn;
    emit isMultipleDotsModeOnChanged(m_isMultipleDotsModeOn);
}

void FretDiagramSettingsModel::setCurrentFretDotType(int currentFretDotType)
{
    FretDiagramTypes::FretDot newFretDotType = static_cast<FretDiagramTypes::FretDot>(currentFretDotType);

    if (m_currentFretDotType == newFretDotType) {
        return;
    }

    m_currentFretDotType = newFretDotType;
    emit currentFretDotTypeChanged(currentFretDotType);
}
