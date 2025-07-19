/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

using namespace mu;
using namespace muse;
using namespace mu::notation;

const QString customPreset()
{
    return muse::qtrc("notation", "Custom");
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

    if (muse::contains(stringTuningsPresets, instrument->id().toStdString())) {
        m_itemId = instrument->id().toStdString();
    } else if (muse::contains(stringTuningsPresets, instrument->family().toStdString())) {
        m_itemId = instrument->family().toStdString();
    }

    const std::vector<engraving::instrString>& stringList = stringData->stringList();
    const std::vector<engraving::string_idx_t>& visibleStrings = stringTunings->visibleStrings();
    int numOfStrings = static_cast<int>(stringList.size());
    for (int i = 0; i < numOfStrings; ++i) {
        engraving::string_idx_t instrStringIndex = numOfStrings - i - 1;
        const engraving::instrString& string = stringList[instrStringIndex];
        StringTuningsItem* item = new StringTuningsItem(this);

        item->blockSignals(true);
        item->setShow(muse::contains(visibleStrings, instrStringIndex));
        item->setNumber(QString::number(i + 1));
        item->setValue(string.pitch);
        item->setUseFlat(string.useFlat);
        item->blockSignals(false);

        m_strings.push_back(item);
    }

    emit numbersOfStringsChanged();
    emit currentNumberOfStringsChanged();
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

    String _stringValue = engraving::convertPitchStringFlatsAndSharpsToUnicode(stringValue);
    int value = engraving::string2pitch(_stringValue);
    if (value == -1) {
        item->valueChanged();
        return false;
    }

    item->setValue(value);

    bool useFlat = _stringValue.contains(u'♭');
    item->setUseFlat(useFlat);

    beginMultiCommands(TranslatableString("undoableAction", "Set string tuning"));

    updateCurrentPreset();
    saveStrings();

    endMultiCommands();

    return true;
}

QString StringTuningsSettingsModel::increaseStringValue(const QString& stringValue)
{
    String value = engraving::convertPitchStringFlatsAndSharpsToUnicode(stringValue);
    return engraving::pitch2string(engraving::string2pitch(value) + 1, false /* useFlats */);
}

QString StringTuningsSettingsModel::decreaseStringValue(const QString& stringValue)
{
    String value = engraving::convertPitchStringFlatsAndSharpsToUnicode(stringValue);
    return engraving::pitch2string(engraving::string2pitch(value) - 1, true /* useFlats */);
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
        int numOfStrings = static_cast<int>(m_strings.size());
        for (int i = 0; i < numOfStrings; ++i) {
            valueList << m_strings.at(numOfStrings - i - 1)->value();
        }

        customMap.insert("value", valueList);

        presetsList << customMap;
    }

    size_t currentStringNumber = this->currentNumberOfStrings();

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
            presetMap.insert("useFlats", preset.useFlats);

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

    beginMultiCommands(TranslatableString("undoableAction", "Load string tunings preset"));

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

QVariantList StringTuningsSettingsModel::numbersOfStrings() const
{
    const InstrumentStringTuningsMap& stringTunings = instrumentsRepository()->stringTuningsPresets();

    QVariantList numbersList;

    if (!contains(stringTunings, m_itemId)) {
        return numbersList;
    }

    for (const StringTuningsInfo& stringTuning : stringTunings.at(m_itemId)) {
        // `lupdate` does not detect the translatable string when the static_cast is in the same line as `qtrc`
        int number = static_cast<int>(stringTuning.number);

        QVariantMap stringNumberMap;
        stringNumberMap.insert("text", muse::qtrc("notation", "%n string(s)", nullptr, number));
        stringNumberMap.insert("value", number);
        numbersList << stringNumberMap;
    }

    return numbersList;
}

int StringTuningsSettingsModel::currentNumberOfStrings() const
{
    return m_item ? engraving::toStringTunings(m_item)->getProperty(engraving::Pid::STRINGTUNINGS_STRINGS_COUNT).toInt() : 0;
}

void StringTuningsSettingsModel::setCurrentNumberOfStrings(int number)
{
    int currentNumber = currentNumberOfStrings();
    if (currentNumber == number) {
        return;
    }

    beginMultiCommands(TranslatableString("undoableAction", "Set number of strings"));

    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_STRINGS_COUNT, number);

    emit currentNumberOfStringsChanged();
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
        if (_preset.toMap()["text"].toString() == currentPreset) {
            QVariantList valueList = _preset.toMap()["value"].toList();
            bool useFlats = _preset.toMap()["useFlats"].toBool();
            int numOfStrings = valueList.size();
            for (int i = 0; i < numOfStrings; ++i) {
                int valueIndex = numOfStrings - i - 1;
                StringTuningsItem* item = new StringTuningsItem(this);

                item->blockSignals(true);
                item->setShow(true);
                item->setNumber(QString::number(i + 1));
                item->setValue(valueList[valueIndex].toInt());
                item->setUseFlat(useFlats);
                item->blockSignals(false);

                m_strings.push_back(item);
            }

            break;
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

    int numOfStrings = m_strings.size();
    for (int i = 0; i < numOfStrings; ++i) {
        const StringTuningsItem* item = m_strings.at(numOfStrings - i - 1);
        stringList[i].pitch = item->value();
        stringList[i].useFlat = item->useFlat();
    }

    beginCommand(TranslatableString("undoableAction", "Edit strings"));

    StringData newStringData(originStringData->frets(), stringList);
    stringTunings->undoStringData(newStringData);

    endCommand();
    updateNotation();
}

void StringTuningsSettingsModel::saveStringsVisibleState()
{
    std::vector<int> visibleStrings;
    int numOfStrings = static_cast<int>(m_strings.size());
    for (int i = 0; i < numOfStrings; ++i) {
        const StringTuningsItem* item = m_strings.at(i);

        if (item->show()) {
            visibleStrings.push_back(numOfStrings - i - 1);
        }
    }

    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_VISIBLE_STRINGS, visibleStrings);
}

void StringTuningsSettingsModel::updateCurrentPreset()
{
    QVariantList currentValueList;
    int numOfStrings = static_cast<int>(m_strings.size());
    for (int i = 0; i < numOfStrings; ++i) {
        currentValueList << m_strings.at(numOfStrings - i - 1)->value();
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
    changeItemProperty(mu::engraving::Pid::STRINGTUNINGS_PRESET, muse::String::fromQString(preset));
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
    return engraving::pitch2string(m_value, m_useFlat).toUpper();
}

void StringTuningsItem::setValue(int value)
{
    if (m_value == value) {
        return;
    }

    m_value = value;
    emit valueChanged();
}

bool StringTuningsItem::useFlat() const
{
    return m_useFlat;
}

void StringTuningsItem::setUseFlat(bool use)
{
    if (m_useFlat == use) {
        return;
    }

    m_useFlat = use;
    emit valueChanged();
}
