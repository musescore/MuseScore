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

#ifndef MU_NOTATION_STRINGTUNINGSSETTINGSMODEL_H
#define MU_NOTATION_STRINGTUNINGSSETTINGSMODEL_H

#include "view/abstractelementpopupmodel.h"

#include "modularity/ioc.h"
#include "iinstrumentsrepository.h"

#include <QObject>

namespace mu::engraving {
class StringTunings;
}

namespace mu::notation {
class StringTuningsItem;
class StringTuningsSettingsModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(QVariantList presets READ presets NOTIFY presetsChanged)
    Q_PROPERTY(QString currentPreset READ currentPreset WRITE setCurrentPreset NOTIFY currentPresetChanged)

    Q_PROPERTY(QVariantList numbersOfStrings READ numbersOfStrings NOTIFY numbersOfStringsChanged)
    Q_PROPERTY(int currentNumberOfStrings READ currentNumberOfStrings WRITE setCurrentNumberOfStrings NOTIFY currentNumberOfStringsChanged)

    Q_PROPERTY(QList<StringTuningsItem*> strings READ strings NOTIFY stringsChanged)

    muse::Inject<IInstrumentsRepository> instrumentsRepository = { this };

public:
    explicit StringTuningsSettingsModel(QObject* parent = nullptr);

    Q_INVOKABLE void init() override;
    Q_INVOKABLE QString pitchToString(int pitch);

    Q_INVOKABLE void toggleString(int stringIndex);
    Q_INVOKABLE bool setStringValue(int stringIndex, const QString& stringValue);

    Q_INVOKABLE QString increaseStringValue(const QString& stringValue);
    Q_INVOKABLE QString decreaseStringValue(const QString& stringValue);

    QVariantList presets(bool withCustom = true) const;

    QString currentPreset() const;
    void setCurrentPreset(const QString& preset);

    QVariantList numbersOfStrings() const;

    int currentNumberOfStrings() const;
    void setCurrentNumberOfStrings(int number);

    QList<StringTuningsItem*> strings() const;

signals:
    void presetsChanged();
    void currentPresetChanged();
    void numbersOfStringsChanged();
    void currentNumberOfStringsChanged();
    void stringsChanged();

private:
    void updateStrings();
    void saveStrings();
    void saveStringsVisibleState();
    void updateCurrentPreset();

    void doSetCurrentPreset(const QString& preset);

    QList<StringTuningsItem*> m_strings;

    std::string m_itemId;
};

class StringTuningsItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool show READ show NOTIFY showChanged)
    Q_PROPERTY(QString number READ number NOTIFY numberChanged)
    Q_PROPERTY(int value READ value NOTIFY valueChanged)
    Q_PROPERTY(QString valueStr READ valueStr NOTIFY valueChanged)
    Q_PROPERTY(bool useFlat READ useFlat WRITE setUseFlat NOTIFY valueChanged)

public:
    explicit StringTuningsItem(QObject* parent = nullptr);

    bool show() const;
    void setShow(bool show);

    QString number() const;
    void setNumber(const QString& number);

    int value() const;
    QString valueStr() const;
    void setValue(int value);

    bool useFlat() const;
    void setUseFlat(bool use);

signals:
    void showChanged();
    void numberChanged();
    void valueChanged();

private:
    bool m_show = false;
    QString m_number;
    int m_value = 0;
    bool m_useFlat = false;
};
} //namespace mu::notation

#endif // MU_NOTATION_STRINGTUNINGSSETTINGSMODEL_H
