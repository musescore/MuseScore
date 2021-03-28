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
#ifndef MU_FRAMEWORK_SETTINGSINFO_H
#define MU_FRAMEWORK_SETTINGSINFO_H

#include <QAbstractListModel>

namespace mu::framework {
class SettingsInfo
{
    Q_GADGET

public:
    enum SettingType {
        Auto,
        NumberSpinner,
        RadioButtonGroup,
        ComboBox
    };

    SettingsInfo() = default;
    SettingsInfo(const char* translatableLabel);
    SettingsInfo(SettingType type, const char* translatableLabel);
    virtual ~SettingsInfo() = default;

    SettingType type() const;
    const char* translatableLabel() const;
    QString translatedLabel() const;
    QVariant toQVariant();

private:
    SettingType m_type = Auto;
    const char* m_translatableLabel = "";
};

using SettingsInfoPtr = std::shared_ptr<SettingsInfo>;

class PickerModel : public QAbstractListModel
{
    Q_OBJECT

public:
    PickerModel(std::vector<std::pair<QVariant, const char*> > options);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    enum Roles {
        TextRole = Qt::UserRole + 1,
        ValueRole
    };

    QVariantList m_values;
    QList<const char*> m_texts;
};

class PickerInfo : public SettingsInfo
{
    Q_GADGET

    Q_PROPERTY(PickerModel * model READ model)

public:
    enum PickerType {
        RadioButtonGroup = SettingsInfo::RadioButtonGroup,
        ComboBox = SettingsInfo::ComboBox
    };

    PickerInfo() = default;
    // Using std::vector<std::pair> instead of map, in order to keep ordering
    PickerInfo(PickerType pickerType, const char* translatableLabel, std::vector<std::pair<QVariant, const char*> > options);
    ~PickerInfo() override;

    PickerModel* model() const;

private:
    PickerModel* m_model = nullptr;
};

class NumberSpinnerInfo : public SettingsInfo
{
    Q_GADGET

    Q_PROPERTY(double minValue READ minValue)
    Q_PROPERTY(double maxValue READ maxValue)
    Q_PROPERTY(double step READ step)
    Q_PROPERTY(int decimals READ decimals)
    Q_PROPERTY(QString measureUnitsSymbol READ measureUnitsSymbol)

public:
    NumberSpinnerInfo() = default;
    NumberSpinnerInfo(const char* translatableLabel, double minValue, double maxValue, double step = 1, int decimals = 0,
                      const char* translatableMeasureUnitsSymbol = "");

    double minValue() const;
    double maxValue() const;
    double step() const;
    int decimals() const;
    QString measureUnitsSymbol() const;

private:
    double m_minValue = 0;
    double m_maxValue = 999;
    double m_step = 1;
    int m_decimals = 0;
    const char* m_translatableMeasureUnitsSymbol = "";
};
}

Q_DECLARE_METATYPE(mu::framework::SettingsInfo)
Q_DECLARE_METATYPE(mu::framework::PickerInfo)
Q_DECLARE_METATYPE(mu::framework::NumberSpinnerInfo)

#endif // MU_FRAMEWORK_SETTINGSINFO_H
