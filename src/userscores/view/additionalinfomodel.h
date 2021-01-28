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
#include "ui/view/musicalsymbolcodes.h"

#include "notation/notationtypes.h"

namespace mu::userscores {
class AdditionalInfoModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap keySignature READ keySignature WRITE setKeySignature NOTIFY keySignatureChanged)

    Q_PROPERTY(QVariantMap timeSignature READ timeSignature NOTIFY timeSignatureChanged)
    Q_PROPERTY(int timeSignatureType READ timeSignatureType WRITE setTimeSignatureType NOTIFY timeSignatureChanged)

    Q_PROPERTY(bool withTempo READ withTempo WRITE setWithTempo NOTIFY withTempoChanged)
    Q_PROPERTY(QVariantMap tempo READ tempo WRITE setTempo NOTIFY tempoChanged)
    Q_PROPERTY(int currentTempoNoteIndex READ currentTempoNoteIndex NOTIFY tempoChanged)

    Q_PROPERTY(QVariantMap pickupTimeSignature READ pickupTimeSignature NOTIFY pickupTimeSignatureChanged)
    Q_PROPERTY(bool withPickupMeasure READ withPickupMeasure WRITE setWithPickupMeasure NOTIFY withPickupMeasureChanged)
    Q_PROPERTY(int measureCount READ measureCount WRITE setMeasureCount NOTIFY measureCountChanged)

public:
    explicit AdditionalInfoModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    Q_INVOKABLE QVariantList keySignatureMajorList() const;
    Q_INVOKABLE QVariantList keySignatureMinorList() const;

    QVariantMap keySignature() const;

    QVariantMap timeSignature() const;
    int timeSignatureType() const;

    Q_INVOKABLE void setTimeSignatureNumerator(int numerator);
    Q_INVOKABLE void setTimeSignatureDenominator(int denominator);

    Q_INVOKABLE QVariantList musicSymbolCodes(int number) const;

    Q_INVOKABLE QVariantList timeSignatureDenominators() const;

    bool withTempo() const;
    QVariantMap tempo() const;
    int currentTempoNoteIndex() const;
    Q_INVOKABLE QVariantMap tempoValueRange() const;
    Q_INVOKABLE QVariantList tempoNotes() const;

    QVariantMap pickupTimeSignature() const;
    Q_INVOKABLE void setPickupTimeSignatureNumerator(int numerator);
    Q_INVOKABLE void setPickupTimeSignatureDenominator(int denominator);
    bool withPickupMeasure() const;
    int measureCount() const;
    Q_INVOKABLE QVariantMap measureCountRange() const;

    enum TimeSignatureType {
        Fraction,
        Common,
        Cut
    };
    Q_ENUMS(TimeSignatureType)

public slots:
    void setKeySignature(QVariantMap keySignature);

    void setTimeSignatureType(int timeSignatureType);

    void setTempo(QVariantMap tempo);
    void setWithTempo(bool withTempo);

    void setWithPickupMeasure(bool withPickupMeasure);
    void setMeasureCount(int measureCount);

signals:
    void keySignatureChanged(QVariantMap keySignature);

    void timeSignatureChanged();
    void pickupTimeSignatureChanged();

    void tempoChanged(QVariantMap tempo);
    void withTempoChanged(bool withTempo);

    void withPickupMeasureChanged(bool withPickupMeasure);
    void measureCountChanged(int measureCount);

private:
    struct KeySignature {
        QString title;
        ui::IconCode::Code icon = ui::IconCode::Code::NONE;
        notation::Key key = notation::Key::C;
        notation::KeyMode mode = notation::KeyMode::UNKNOWN;

        KeySignature() = default;
        KeySignature(const QVariantMap& map);
        KeySignature(const QString& title, ui::IconCode::Code icon, notation::Key key, notation::KeyMode mode);

        QVariantMap toMap() const;
    };

    struct Tempo {
        int value = 0;
        ui::MusicalSymbolCodes::Code noteIcon = ui::MusicalSymbolCodes::Code::NONE;
        bool withDot = false;

        Tempo() = default;
        Tempo(const QVariantMap& map);

        QVariantMap toMap() const;
    };

    void setTimeSignature(const notation::Fraction& timeSignature);
    void setPickupTimeSignature(const notation::Fraction& pickupTimeSignature);

    KeySignature m_keySignature;

    notation::Fraction m_timeSignature;
    bool m_timeCommon = false;
    bool m_timeCut = false;
    bool m_timeFraction = false;

    Tempo m_tempo;
    bool m_withTempo = false;

    notation::Fraction m_pickupTimeSignature;
    bool m_withPickupMeasure = false;
    int m_measureCount = 0;
    notation::TimeSigType m_timeSignatureType = notation::TimeSigType::NORMAL;
};
}

#endif // MU_USERSCORES_ADDITIONALINFOMODEL_H
