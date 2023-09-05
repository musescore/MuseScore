/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "stringtuningssettingsmodel.h"

#include "engraving/dom/stringtunings.h"
#include "engraving/dom/utils.h"

#include "translation.h"

using namespace mu::notation;

const QString customPreset()
{
    return qtrc("notation", "Custom");
}

StringTuningsSettingsModel::StringTuningsSettingsModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_STRING_TUNINGS, parent)
{
}

void StringTuningsSettingsModel::init()
{
    TRACEFUNC;

    m_strings.clear();

    AbstractElementPopupModel::init();

    IF_ASSERT_FAILED(m_item || m_item->isStringTunings()) {
        return;
    }

    engraving::StringTunings* stringTunings = engraving::toStringTunings(m_item);
    const mu::engraving::StringData* stringData = stringTunings->stringData();
    IF_ASSERT_FAILED(stringData) {
        return;
    }

    const mu::engraving::Part* part = m_item->part();
    IF_ASSERT_FAILED(part) {
        return;
    }

    const mu::engraving::Instrument* instrument = part->instrument(m_item->tick());
    IF_ASSERT_FAILED(instrument) {
        return;
    }

    const InstrumentStringTuningsMap& stringTuningsPresets = instrumentsRepository()->stringTuningsPresets();

    if (contains(stringTuningsPresets, instrument->id().toStdString())) {
        m_itemId = instrument->id().toStdString();
    } else if (contains(stringTuningsPresets, instrument->family().toStdString())) {
        m_itemId = instrument->family().toStdString();
    }

    const std::vector<engraving::instrString>& stringList = stringData->stringList();
    std::vector<engraving::string_idx_t> visibleStrings = stringTunings->visibleStrings();
    for (engraving::string_idx_t i = 0; i < stringList.size(); ++i) {
        const engraving::instrString& string = stringList[i];
        StringTuningsItem* item = new StringTuningsItem(this);

        item->blockSignals(true);
        item->setShow(contains(visibleStrings, i));
        item->setNumber(QString::number(i + 1)); // todo
        item->setValue(string.pitch);
        item->blockSignals(false);

        m_strings.push_back(item);
    }

    emit stringNumbersChanged();
    emit currentStringNumberChanged();
    emit presetsChanged();
    emit currentPresetChanged();
    emit stringsChanged();
}

QString StringTuningsSettingsModel::pitchToString(int pitch)
{
    return engraving::pitch2string(pitch);
}

void StringTuningsSettingsModel::toggleString(int stringIndex)
{
    if (stringIndex >= m_strings.size()) {
        return;
    }

    StringTuningsItem* item = m_strings.at(stringIndex);
    item->setShow(!item->show());

    saveStringsVisibleState();
}

bool StringTuningsSettingsModel::setStringValue(int stringIndex, const QString& stringValue)
{
    if (stringIndex >= m_strings.size()) {
        return false;
    }

    StringTuningsItem* item = m_strings.at(stringIndex);

    int value = engraving::string2pitch(stringValue);
    if (value == -1) {
        item->valueChanged();
        return false;
    }

    item->setValue(value);

    beginMultiCommands();

    updateCurrentPreset();
    saveStrings();

    endMultiCommands();

    return true;
}

bool StringTuningsSettingsModel::canIncreaseStringValue(const QString& stringValue) const
{
    return engraving::string2pitch(stringValue) != -1;
}

QString StringTuningsSettingsModel::increaseStringValue(const QString& stringValue)
{
    return engraving::pitch2string(engraving::string2pitch(stringValue) + 1);
}

bool StringTuningsSettingsModel::canDecreaseStringValue(const QString& stringValue) const
{
    return engraving::string2pitch(stringValue) != -1;
}

QString StringTuningsSettingsModel::decreaseStringValue(const QString& stringValue)
{
    return engraving::pitch2string(engraving::string2pitch(stringValue) - 1);
}

QVariantList StringTuningsSettingsModel::presets(bool withCustom) const
{
    const InstrumentStringTuningsMap& stringTunings = instrumentsRepository()->stringTuningsPresets();

    QVariantList presetsList;

    if (!contains(stringTunings, m_itemId)) {
        return presetsList;
    }

    QString custom = customPreset();
    if (withCustom && (currentPreset().isEmpty() || currentPreset() == custom)) {
        QVariantMap customMap;
        customMap.insert("text", custom);

        QVariantList valueList;
        for (const StringTuningsItem* item : m_strings) {
            valueList << item->value();
        }

        customMap.insert("value", valueList);

        presetsList << customMap;
    }

    int currentStringNumber = this->currentStringNumber();

    for (const StringTuningsInfo& stringTuning : stringTunings.at(m_itemId)) {
        if (stringTuning.number != currentStringNumber) {
            continue;
        }

        for (const StringTuningPreset& preset : stringTuning.presets) {
            QVariantMap presetMap;
            presetMap.insert("text", QString::fromStdString(preset.name));

            QVariantList valueList;
            for (int value : preset.value) {
                valueList << value;
            }

            presetMap.insert("value", valueList);

            presetsList.push_back(presetMap);
        }
    }

    return presetsList;
}

QString StringTuningsSettingsModel::currentPreset() const
{
    QString preset
        = m_item ? engraving::toStringTunings(m_item)->getProperty(engraving::Pid::STRINGTUNINGS_PRESET).value<String>().toQString() : "";

    if (preset.isEmpty()) {
        preset = customPreset();
    }

    return preset;
}

void StringTuningsSettingsModel::setCurrentPreset(const QString& preset)
{
    bool isCurrentCustom = currentPreset() == customPreset();

    beginMultiCommands();

    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_PRESET, String::fromQString(preset));
    emit currentPresetChanged();

    if (isCurrentCustom) {
        //! NOTE: if current preset was custom then we should update presets.
        //! Custom preset will no longer be available
        emit presetsChanged();
    }

    updateStrings();
    saveStrings();
    saveStringsVisibleState();

    endMultiCommands();
}

QVariantList StringTuningsSettingsModel::stringNumbers() const
{
    const InstrumentStringTuningsMap& stringTunings = instrumentsRepository()->stringTuningsPresets();

    QVariantList numbersList;

    if (!contains(stringTunings, m_itemId)) {
        return numbersList;
    }

    for (const StringTuningsInfo& stringTuning : stringTunings.at(m_itemId)) {
        QVariantMap stringNumberMap;
        stringNumberMap.insert("text", QString::number(stringTuning.number) + " " + tr("strings"));
        stringNumberMap.insert("value", stringTuning.number);
        numbersList << stringNumberMap;
    }

    return numbersList;
}

int StringTuningsSettingsModel::currentStringNumber() const
{
    return m_item ? engraving::toStringTunings(m_item)->getProperty(engraving::Pid::STRINGTUNINGS_STRINGS_COUNT).toInt() : 0;
}

void StringTuningsSettingsModel::setCurrentStringNumber(int stringNumber)
{
    if (currentStringNumber() == stringNumber) {
        return;
    }

    beginMultiCommands();

    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_STRINGS_COUNT, stringNumber);

    emit currentStringNumberChanged();
    emit presetsChanged();

    setCurrentPreset(presets(false /*withCustom*/).first().toMap()["text"].toString());

    emit stringsChanged();

    endMultiCommands();
}

QList<StringTuningsItem*> StringTuningsSettingsModel::strings() const
{
    return m_strings;
}

void StringTuningsSettingsModel::updateStrings()
{
    const QVariantList presets = this->presets();
    QString currentPreset = this->currentPreset();

    m_strings.clear();

    for (const QVariant& _preset : presets) {
        if (_preset.toMap()["text"].toString() != currentPreset) {
            continue;
        }

        QVariantList valueList = _preset.toMap()["value"].toList();
        for (int i = 0; i < valueList.size(); ++i) {
            StringTuningsItem* item = new StringTuningsItem(this);

            item->blockSignals(true);
            item->setShow(true);
            item->setNumber(QString::number(i + 1)); // todo
            item->setValue(valueList[i].toInt());
            item->blockSignals(false);

            m_strings.push_back(item);
        }
    }

    emit stringsChanged();
}

void StringTuningsSettingsModel::saveStrings()
{
    engraving::StringTunings* stringTunings = engraving::toStringTunings(m_item);
    IF_ASSERT_FAILED(stringTunings) {
        return;
    }

    const mu::engraving::StringData* originStringData = stringTunings->stringData();

    std::vector<engraving::instrString> stringList = originStringData->stringList();
    stringList.resize(m_strings.size());

    for (int i = 0; i < m_strings.size(); ++i) {
        stringList[i].open = false;
        stringList[i].pitch = m_strings[i]->value();
    }

    beginCommand();

    StringData newStringData(originStringData->frets(), stringList);
    stringTunings->undoStringData(newStringData);

    endCommand();
    updateNotation();
}

void StringTuningsSettingsModel::saveStringsVisibleState()
{
    std::vector<int> visibleStrings;
    for (int i = 0; i < m_strings.size(); ++i) {
        const StringTuningsItem* item = m_strings.at(i);

        if (item->show()) {
            visibleStrings.push_back(i);
        }
    }

    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_VISIBLE_STRINGS, visibleStrings);
}

void StringTuningsSettingsModel::updateCurrentPreset()
{
    QVariantList currentValueList;
    for (int i = 0; i < m_strings.size(); ++i) {
        currentValueList << m_strings[i]->value();
    }

    const QVariantList presets = this->presets(false /*withCustom*/);
    QString currentPreset = this->currentPreset();
    QString newPreset;
    for (const QVariant& _preset : presets) {
        if (currentValueList == _preset.toMap()["value"].toList()) {
            newPreset = _preset.toMap()["text"].toString();
            break;
        }
    }

    if (currentPreset != newPreset) {
        if (newPreset.isEmpty()) {
            newPreset = customPreset();
        }

        doSetCurrentPreset(newPreset);
        emit presetsChanged();
    }
}

void StringTuningsSettingsModel::doSetCurrentPreset(const QString& preset)
{
    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_PRESET, String::fromQString(preset));
    emit currentPresetChanged();
}

StringTuningsItem::StringTuningsItem(QObject* parent)
    : QObject(parent)
{
}

bool StringTuningsItem::show() const
{
    return m_show;
}

void StringTuningsItem::setShow(bool show)
{
    if (m_show == show) {
        return;
    }

    m_show = show;
    emit showChanged();
}

QString StringTuningsItem::number() const
{
    return m_number;
}

void StringTuningsItem::setNumber(const QString& number)
{
    if (m_number == number) {
        return;
    }

    m_number = number;
    emit numberChanged();
}

int StringTuningsItem::value() const
{
    return m_value;
}

QString StringTuningsItem::valueStr() const
{
    return engraving::pitch2string(m_value).toUpper();
}

void StringTuningsItem::setValue(int value)
{
    if (m_value == value) {
        return;
    }

    m_value = value;
    emit valueChanged();
}
