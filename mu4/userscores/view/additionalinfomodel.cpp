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
#include "additionalinfomodel.h"

#include "libmscore/sym.h"

#include "log.h"
#include "translation.h"
#include "ui/view/iconcodes.h"
#include "ui/view/musicalsymbolcodes.h"

using namespace mu::userscores;
using namespace mu::framework;
using namespace mu::notation;

AdditionalInfoModel::AdditionalInfoModel(QObject* parent)
    : QObject(parent)
{
}

void AdditionalInfoModel::init()
{
    setKeySignature(KeySignature(qtrc("userscore", "None"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C, KeyMode::MAJOR).toMap());

    setTimeSignatureType(static_cast<int>(TimeSignatureType::Common));
    setTimeSignature(TimeSignature(4, 4).toMap());

    setWithTempo(false);
    setTempo(120);

    setWithPickupMeasure(false);
    setMeasureCount(32);
    setPickupTimeSignature(TimeSignature(4, 4).toMap());
}

QVariantList AdditionalInfoModel::keySignatureMajorList()
{
    QVariantList majorList;
    majorList << KeySignature(qtrc("userscore", "C major"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "F major"), IconCode::Code::KEY_SIGNATURE_1_FLAT, Key::F, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "Bb major"), IconCode::Code::KEY_SIGNATURE_2_FLAT, Key::B_B, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "Eb major"), IconCode::Code::KEY_SIGNATURE_3_FLAT, Key::E_B, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "Ab major"), IconCode::Code::KEY_SIGNATURE_4_FLAT, Key::A_B, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "Db major"), IconCode::Code::KEY_SIGNATURE_5_FLAT, Key::D_B, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "Gb major"), IconCode::Code::KEY_SIGNATURE_6_FLAT, Key::G_B, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "Cb major"), IconCode::Code::KEY_SIGNATURE_7_FLAT, Key::C_B, KeyMode::MAJOR).toMap()

              << KeySignature(qtrc("userscore", "None"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "G major"), IconCode::Code::KEY_SIGNATURE_1_SHARP, Key::G, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "D major"), IconCode::Code::KEY_SIGNATURE_2_SHARPS, Key::D, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "A major"), IconCode::Code::KEY_SIGNATURE_3_SHARPS, Key::A, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "E major"), IconCode::Code::KEY_SIGNATURE_4_SHARPS, Key::E, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "B major"), IconCode::Code::KEY_SIGNATURE_5_SHARPS, Key::B, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "F# major"), IconCode::Code::KEY_SIGNATURE_6_SHARPS, Key::F_S, KeyMode::MAJOR).toMap()
              << KeySignature(qtrc("userscore", "C# major"), IconCode::Code::KEY_SIGNATURE_7_SHARPS, Key::C_S, KeyMode::MAJOR).toMap();

    return majorList;
}

QVariantList AdditionalInfoModel::keySignatureMinorList()
{
    QVariantList minorList;
    minorList << KeySignature(qtrc("userscore", "A minor"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "D minor"), IconCode::Code::KEY_SIGNATURE_1_FLAT, Key::F, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "G minor"), IconCode::Code::KEY_SIGNATURE_2_FLAT, Key::B_B, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "C minor"), IconCode::Code::KEY_SIGNATURE_3_FLAT, Key::E_B, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "F minor"), IconCode::Code::KEY_SIGNATURE_4_FLAT, Key::A_B, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "Bb minor"), IconCode::Code::KEY_SIGNATURE_5_FLAT, Key::D_B, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "Eb minor"), IconCode::Code::KEY_SIGNATURE_6_FLAT, Key::G_B, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "Ab minor"), IconCode::Code::KEY_SIGNATURE_7_FLAT, Key::C_B, KeyMode::MINOR).toMap()

              << KeySignature(qtrc("userscore", "None"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "E minor"), IconCode::Code::KEY_SIGNATURE_1_SHARP, Key::G, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "B minor"), IconCode::Code::KEY_SIGNATURE_2_SHARPS, Key::D, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "F# minor"), IconCode::Code::KEY_SIGNATURE_3_SHARPS, Key::A, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "C# minor"), IconCode::Code::KEY_SIGNATURE_4_SHARPS, Key::E, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "G# minor"), IconCode::Code::KEY_SIGNATURE_5_SHARPS, Key::B, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "D# minor"), IconCode::Code::KEY_SIGNATURE_6_SHARPS, Key::F_S, KeyMode::MINOR).toMap()
              << KeySignature(qtrc("userscore", "A# minor"), IconCode::Code::KEY_SIGNATURE_7_SHARPS, Key::C_S, KeyMode::MINOR).toMap();

    return minorList;
}

QVariantMap AdditionalInfoModel::keySignature() const
{
    return m_keySignature.toMap();
}

QVariantMap AdditionalInfoModel::timeSignature() const
{
    return m_timeSignature.toMap();
}

int AdditionalInfoModel::timeSignatureType() const
{
    return static_cast<int>(m_timeSignatureType);
}

void AdditionalInfoModel::setTimeSignatureNumerator(int numerator)
{
    m_timeSignature.numerator = numerator;
    setTimeSignature(m_timeSignature.toMap());
}

void AdditionalInfoModel::setTimeSignatureDenominator(int denominator)
{
    m_timeSignature.denominator = denominator;
    setTimeSignature(m_timeSignature.toMap());
}

QVariantList AdditionalInfoModel::musicSymbolCodes(int number) const
{
    static QMap<QString, MusicalSymbolCodes::Code> numeralsMusicSymbolCodes = {
        { "0", MusicalSymbolCodes::Code::ZERO },
        { "1", MusicalSymbolCodes::Code::ONE },
        { "2", MusicalSymbolCodes::Code::TWO },
        { "3", MusicalSymbolCodes::Code::THREE },
        { "4", MusicalSymbolCodes::Code::FOUR },
        { "5", MusicalSymbolCodes::Code::FIVE },
        { "6", MusicalSymbolCodes::Code::SIX },
        { "7", MusicalSymbolCodes::Code::SEVEN },
        { "8", MusicalSymbolCodes::Code::EIGHT },
        { "9", MusicalSymbolCodes::Code::NINE },
    };

    QVariantList result;
    QString numberStr = QString::number(number);
    for (const QString& numeral: numberStr) {
        result << static_cast<int>(numeralsMusicSymbolCodes[numeral]);
    }

    return result;
}

int AdditionalInfoModel::tempo() const
{
    return m_tempo;
}

bool AdditionalInfoModel::withTempo() const
{
    return m_withTempo;
}

QVariantMap AdditionalInfoModel::tempoRange()
{
    return QVariantMap { { "min", 20 }, { "max", 400 } };
}

QVariantList AdditionalInfoModel::tempoMarks()
{
    QVariantList marks;
    marks << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::SEMIQUAVER) } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::SEMIQUAVER) }, { "withDot", true } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::CROTCHET) } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::CROTCHET) }, { "withDot", true } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::QUAVER) } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::QUAVER) }, { "withDot", true } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::MINIM) } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::MINIM) }, { "withDot", true } }
          << QVariantMap { { "icon", static_cast<int>(MusicalSymbolCodes::Code::SEMIBREVE) } };

    return marks;
}

QVariantMap AdditionalInfoModel::pickupTimeSignature() const
{
    return m_pickupTimeSignature.toMap();
}

void AdditionalInfoModel::setPickupTimeSignatureNumerator(int numerator)
{
    m_pickupTimeSignature.numerator = numerator;
    setPickupTimeSignature(m_pickupTimeSignature.toMap());
}

void AdditionalInfoModel::setPickupTimeSignatureDenominator(int denominator)
{
    m_pickupTimeSignature.denominator = denominator;
    setPickupTimeSignature(m_pickupTimeSignature.toMap());
}

bool AdditionalInfoModel::withPickupMeasure() const
{
    return m_withPickupMeasure;
}

int AdditionalInfoModel::measureCount() const
{
    return m_measureCount;
}

QVariantMap AdditionalInfoModel::measureCountRange()
{
    return QVariantMap { { "min", 1 }, { "max", 9999 } };
}

QVariantList AdditionalInfoModel::timeSignatureDenominators()
{
    return QVariantList { 1, 2, 4, 8, 16, 32, 64 };
}

void AdditionalInfoModel::setKeySignature(QVariantMap keySignature)
{
    m_keySignature = KeySignature(keySignature);
    emit keySignatureChanged(keySignature);
}

void AdditionalInfoModel::setTimeSignature(QVariantMap timeSignature)
{
    m_timeSignature = TimeSignature(timeSignature);
    emit timeSignatureChanged(timeSignature);
}

void AdditionalInfoModel::setTimeSignatureType(int timeSignatureType)
{
    TimeSigType type = static_cast<TimeSigType>(timeSignatureType);
    if (m_timeSignatureType == type) {
        return;
    }

    m_timeSignatureType = type;
    emit timeSignatureTypeChanged(timeSignatureType);

    updateTimeSignature();
}

void AdditionalInfoModel::setTempo(int tempo)
{
    if (m_tempo == tempo) {
        return;
    }

    m_tempo = tempo;
    emit tempoChanged(m_tempo);
}

void AdditionalInfoModel::setWithTempo(bool withTempo)
{
    if (m_withTempo == withTempo) {
        return;
    }

    m_withTempo = withTempo;
    emit withTempoChanged(m_withTempo);
}

void AdditionalInfoModel::setPickupTimeSignature(QVariantMap pickupTimeSignature)
{
    m_pickupTimeSignature = TimeSignature(pickupTimeSignature);
    emit pickupTimeSignatureChanged(pickupTimeSignature);
}

void AdditionalInfoModel::setWithPickupMeasure(bool withPickupMeasure)
{
    if (m_withPickupMeasure == withPickupMeasure) {
        return;
    }

    m_withPickupMeasure = withPickupMeasure;
    emit withPickupMeasureChanged(m_withPickupMeasure);
}

void AdditionalInfoModel::setMeasureCount(int measureCount)
{
    if (m_measureCount == measureCount) {
        return;
    }

    m_measureCount = measureCount;
    emit measureCountChanged(m_measureCount);
}

void AdditionalInfoModel::updateTimeSignature()
{
    switch (m_timeSignatureType) {
    case TimeSigType::FOUR_FOUR:
        setTimeSignature(TimeSignature(4, 4).toMap());
        break;
    case TimeSigType::ALLA_BREVE:
        setTimeSignature(TimeSignature(2, 2).toMap());
        break;
    case TimeSigType::NORMAL:
        break;
    }
}
