//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_USERSCORES_ADDITIONALINFOMODEL_H
#define MU_USERSCORES_ADDITIONALINFOMODEL_H

#include <QObject>

#include "ui/view/iconcodes.h"
#include "notation/notationtypes.h"

namespace mu {
namespace userscores {
class AdditionalInfoModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap keySignature READ keySignature WRITE setKeySignature NOTIFY keySignatureChanged)

    Q_PROPERTY(QVariantMap timeSignature READ timeSignature WRITE setTimeSignature NOTIFY timeSignatureChanged)
    Q_PROPERTY(int timeSignatureType READ timeSignatureType WRITE setTimeSignatureType NOTIFY timeSignatureTypeChanged)

    Q_PROPERTY(int tempo READ tempo WRITE setTempo NOTIFY tempoChanged)
    Q_PROPERTY(bool withTempo READ withTempo WRITE setWithTempo NOTIFY withTempoChanged)

    Q_PROPERTY(QVariantMap pickupTimeSignature READ pickupTimeSignature WRITE setPickupTimeSignature NOTIFY pickupTimeSignatureChanged)
    Q_PROPERTY(bool withPickupMeasure READ withPickupMeasure WRITE setWithPickupMeasure NOTIFY withPickupMeasureChanged)
    Q_PROPERTY(int measureCount READ measureCount WRITE setMeasureCount NOTIFY measureCountChanged)

public:
    explicit AdditionalInfoModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    Q_INVOKABLE QVariantList keySignatureMajorList();
    Q_INVOKABLE QVariantList keySignatureMinorList();

    QVariantMap keySignature() const;

    QVariantMap timeSignature() const;
    int timeSignatureType() const;
    Q_INVOKABLE void setTimeSignatureNumerator(int numerator);
    Q_INVOKABLE void setTimeSignatureDenominator(int denominator);

    Q_INVOKABLE QVariantList musicSymbolCodes(int number) const;

    Q_INVOKABLE QVariantList timeSignatureDenominators();

    int tempo() const;
    bool withTempo() const;
    Q_INVOKABLE QVariantMap tempoRange();
    Q_INVOKABLE QVariantList tempoMarks();

    QVariantMap pickupTimeSignature() const;
    Q_INVOKABLE void setPickupTimeSignatureNumerator(int numerator);
    Q_INVOKABLE void setPickupTimeSignatureDenominator(int denominator);
    bool withPickupMeasure() const;
    int measureCount() const;
    Q_INVOKABLE QVariantMap measureCountRange();

    enum TimeSignatureType {
        Fraction,
        Common,
        Cut
    };
    Q_ENUMS(TimeSignatureType)

public slots:
    void setKeySignature(QVariantMap keySignature);

    void setTimeSignature(QVariantMap timeSignature);
    void setTimeSignatureType(int timeSignatureType);

    void setTempo(int tempo);
    void setWithTempo(bool withTempo);

    void setPickupTimeSignature(QVariantMap pickupTimeSignature);
    void setWithPickupMeasure(bool withPickupMeasure);
    void setMeasureCount(int measureCount);

signals:
    void keySignatureChanged(QVariantMap keySignature);

    void timeSignatureChanged(QVariantMap timeSignature);
    void timeSignatureTypeChanged(int timeSignatureType);

    void tempoChanged(int tempo);
    void withTempoChanged(bool withTempo);

    void pickupTimeSignatureChanged(QVariantMap pickupTimeSignature);
    void withPickupMeasureChanged(bool withPickupMeasure);
    void measureCountChanged(int measureCount);

private:
    struct KeySignature {
        QString title;
        framework::IconCode::Code icon = framework::IconCode::Code::NONE;
        notation::Key key = notation::Key::C;
        notation::KeyMode mode = notation::KeyMode::UNKNOWN;

        QVariantMap toMap() const
        {
            return {
                { "title", title },
                { "icon", static_cast<int>(icon) },
                { "key", static_cast<int>(key) },
                { "mode", static_cast<int>(mode) }
            };
        }

        KeySignature() = default;

        KeySignature(const QString& title, const framework::IconCode::Code& icon, const notation::Key& key,
                     const notation::KeyMode& mode)
            : title(title), icon(icon), key(key), mode(mode) {}

        KeySignature(const QVariantMap& map)
        {
            title = map["title"].toString();
            icon = static_cast<framework::IconCode::Code>(map["icon"].toInt());
            key = static_cast<Ms::Key>(map["key"].toInt());
            mode = static_cast<Ms::KeyMode>(map["mode"].toInt());
        }
    };

    struct TimeSignature {
        int numerator = 0;
        int denominator = 1;

        QVariantMap toMap() const
        {
            return {
                { "numerator", numerator },
                { "denominator", denominator }
            };
        }

        TimeSignature() = default;

        TimeSignature(int numetator, int denominator)
            : numerator(numetator), denominator(denominator) {}

        TimeSignature(const QVariantMap& map)
        {
            numerator = map["numerator"].toInt();
            denominator = map["denominator"].toInt();
        }
    };

    void updateTimeSignature();

    KeySignature m_keySignature;

    TimeSignature m_timeSignature;
    bool m_timeCommon = false;
    bool m_timeCut = false;
    bool m_timeFraction = false;

    int m_tempo = 0;
    bool m_withTempo = false;

    TimeSignature m_pickupTimeSignature;
    bool m_withPickupMeasure = false;
    int m_measureCount = 0;
    notation::TimeSigType m_timeSignatureType;
};
}
}

#endif // MU_USERSCORES_ADDITIONALINFOMODEL_H
