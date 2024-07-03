/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "fretdiagramsettingsmodel.h"

#include "dataformatter.h"
#include "translation.h"

#include "engraving/dom/fret.h"

using namespace mu::inspector;

FretDiagramSettingsModel::FretDiagramSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setModelType(InspectorModelType::TYPE_FRET_DIAGRAM);
    setTitle(muse::qtrc("inspector", "Fretboard diagram"));
    setIcon(muse::ui::IconCode::Code::FRETBOARD_DIAGRAM);
    createProperties();
}

void FretDiagramSettingsModel::createProperties()
{
    m_scale = buildPropertyItem(mu::engraving::Pid::MAG,
                                [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toDouble() / 100);
    },
                                [this](const mu::engraving::Sid sid, const QVariant& newValue) {
        updateStyleValue(sid, newValue.toDouble() / 100);
        emit requestReloadPropertyItems();
    });

    m_stringsCount = buildPropertyItem(mu::engraving::Pid::FRET_STRINGS, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);
        loadPropertyItem(m_fingerings);

        emit fretDiagramChanged(fretDiagram());
    });

    m_fretsCount = buildPropertyItem(mu::engraving::Pid::FRET_FRETS, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit fretDiagramChanged(fretDiagram());
    });

    m_fretNumber = buildPropertyItem(mu::engraving::Pid::FRET_OFFSET, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue.toInt() - 1);

        emit fretDiagramChanged(fretDiagram());
    });

    m_isNutVisible = buildPropertyItem(mu::engraving::Pid::FRET_NUT, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit fretDiagramChanged(fretDiagram());
    });

    m_placement = buildPropertyItem(mu::engraving::Pid::PLACEMENT);
    m_orientation = buildPropertyItem(mu::engraving::Pid::ORIENTATION);

    m_showFingerings
        = buildPropertyItem(mu::engraving::Pid::FRET_SHOW_FINGERINGS, [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        onPropertyValueChanged(pid, newValue);

        emit fretDiagramChanged(fretDiagram());
    });

    m_fingerings = buildPropertyItem(mu::engraving::Pid::FRET_FINGERING, [this](const mu::engraving::Pid pid, const QVariant& newValue){
        onPropertyValueChanged(pid, newValue);

        emit fretDiagramChanged(fretDiagram());
    });

    connect(m_fingerings, &PropertyItem::valueChanged, this, [this]() {
        emit fingeringsChanged(fingerings());
    });
}

void FretDiagramSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::FRET_DIAGRAM);

    emit fretDiagramChanged(fretDiagram());
    emit areSettingsAvailableChanged(areSettingsAvailable());
}

void FretDiagramSettingsModel::loadProperties()
{
    loadPropertyItem(m_scale, [](const QVariant& elementPropertyValue) -> QVariant {
        return muse::DataFormatter::roundDouble(elementPropertyValue.toDouble()) * 100;
    });

    loadPropertyItem(m_stringsCount);
    loadPropertyItem(m_fretsCount);
    loadPropertyItem(m_fretNumber, [](const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toInt() + 1;
    });

    loadPropertyItem(m_isNutVisible);
    loadPropertyItem(m_placement);
    loadPropertyItem(m_orientation);
    loadPropertyItem(m_showFingerings);
    loadPropertyItem(m_fingerings);
    emit fingeringsChanged(fingerings());
}

void FretDiagramSettingsModel::resetProperties()
{
    m_scale->resetToDefault();
    m_stringsCount->resetToDefault();
    m_fretsCount->resetToDefault();
    m_fretNumber->resetToDefault();
    m_isNutVisible->resetToDefault();
    m_placement->resetToDefault();
    m_showFingerings->resetToDefault();
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

PropertyItem* FretDiagramSettingsModel::fretNumber() const
{
    return m_fretNumber;
}

PropertyItem* FretDiagramSettingsModel::isNutVisible() const
{
    return m_isNutVisible;
}

PropertyItem* FretDiagramSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* FretDiagramSettingsModel::orientation() const
{
    return m_orientation;
}

PropertyItem* FretDiagramSettingsModel::showFingerings() const
{
    return m_showFingerings;
}

QStringList FretDiagramSettingsModel::fingerings() const
{
    QString fingerings = m_fingerings->value().value<QString>();
    return fingerings.split(',');
}

void FretDiagramSettingsModel::setFingering(int string, int finger)
{
    finger = std::clamp(finger, 0, 5);

    QStringList curFingerings = fingerings();
    assert(string < curFingerings.size());

    QString newFinger = QString::number(finger);
    curFingerings[string] = newFinger;
    QString newFingerings = curFingerings.join(",");

    m_fingerings->setValue(newFingerings);
}

void FretDiagramSettingsModel::resetFingerings()
{
    m_fingerings->resetToDefault();
}

QVariant FretDiagramSettingsModel::fretDiagram() const
{
    if (m_elementList.isEmpty()) {
        return QVariant();
    }

    return QVariant::fromValue(mu::engraving::toFretDiagram(m_elementList.first()));
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
