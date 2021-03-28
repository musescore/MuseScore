//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "settingsinfo.h"
#include "translation.h"

using namespace mu::framework;

SettingsInfo::SettingsInfo(const char* translatableLabel)
    : m_translatableLabel(translatableLabel)
{
}

SettingsInfo::SettingsInfo(SettingType type, const char* translatableLabel)
    : m_type(type), m_translatableLabel(translatableLabel)
{
}

SettingsInfo::SettingType SettingsInfo::type() const
{
    return m_type;
}

const char* SettingsInfo::translatableLabel() const
{
    return m_translatableLabel;
}

QString SettingsInfo::translatedLabel() const
{
    return qtrc("settings", m_translatableLabel);
}

QVariant SettingsInfo::toQVariant()
{
    switch (type()) {
    case Auto:
        return QVariant::fromValue(*this);
    case NumberSpinner:
        return QVariant::fromValue(*dynamic_cast<NumberSpinnerInfo*>(this));
    case RadioButtonGroup:
    case ComboBox:
        return QVariant::fromValue(*dynamic_cast<PickerInfo*>(this));
    }

    return QVariant();
}

//=============================================================================

PickerInfo::PickerInfo(PickerType pickerType, const char* translatableLabel,
                       std::vector<std::pair<QVariant, const char*> > options)
    : SettingsInfo(static_cast<SettingType>(pickerType), translatableLabel)
{
    m_model = new PickerModel(options);
}

PickerInfo::~PickerInfo()
{
    if (m_model) {
//        delete m_model;
    }
}

PickerModel* PickerInfo::model() const
{
    return m_model;
}

PickerModel::PickerModel(std::vector<std::pair<QVariant, const char*> > options)
    : QAbstractListModel()
{
    for (auto pair : options) {
        m_values << pair.first;
        m_texts << pair.second;
    }
}

QVariant PickerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() > rowCount()) {
        return QVariant();
    }

    switch (role) {
    case TextRole:
        return qtrc("settings", m_texts[index.row()]);
    case ValueRole:
        return m_values[index.row()];
    }

    return QVariant();
}

int PickerModel::rowCount(const QModelIndex&) const
{
    IF_ASSERT_FAILED(m_values.size() == m_texts.size()) {
        return std::min(m_values.size(), m_texts.size());
    }

    return m_values.size();
}

QHash<int, QByteArray> PickerModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { TextRole, "textRole" },
        { ValueRole, "valueRole" },
    };

    return roles;
}

NumberSpinnerInfo::NumberSpinnerInfo(const char* translatableLabel,
                                     double minValue, double maxValue, double step, int decimals,
                                     const char* translatableMeasureUnitsSymbol)
    : SettingsInfo(NumberSpinner, translatableLabel)
    , m_minValue(minValue), m_maxValue(maxValue), m_step(step), m_decimals(decimals)
    , m_translatableMeasureUnitsSymbol(translatableMeasureUnitsSymbol)
{
}

double NumberSpinnerInfo::minValue() const
{
    return m_minValue;
}

double NumberSpinnerInfo::maxValue() const
{
    return m_maxValue;
}

double NumberSpinnerInfo::step() const
{
    return m_step;
}

int NumberSpinnerInfo::decimals() const
{
    return m_decimals;
}

QString NumberSpinnerInfo::measureUnitsSymbol() const
{
    return qtrc("settings", m_translatableMeasureUnitsSymbol);
}
