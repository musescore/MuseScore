/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "additionalinfomodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::ui;

static const mu::notation::Fraction FOUR_FOUR_TIME_SIGNATURE(4, 4);
static const mu::notation::Fraction ALLA_BREVE_TIME_SIGNATURE(2, 2);
static const mu::notation::Fraction CUT_BACH_TIME_SIGNATURE(2, 2);
static const mu::notation::Fraction CUT_TRIPLE_TIME_SIGNATURE(9, 8);
static const mu::notation::Fraction DEFAULT_PICKUP_TIME_SIGNATURE(1, 4);

static const QString VALUE_KEY("value");
static const QString NOTE_ICON_KEY("noteIcon");
static const QString NOTE_SYMBOL_KEY("noteSymbol");
static const QString TITLE_MAJOR_KEY("titleMajor");
static const QString TITLE_MINOR_KEY("titleMinor");
static const QString ICON_KEY("icon");
static const QString SIGNATURE_KEY("key");
static const QString WITH_DOT_KEY("withDot");
static const QString MIN_KEY("min");
static const QString MAX_KEY("max");

using MusicalSymbolCode = MusicalSymbolCodes::Code;

AdditionalInfoModel::KeySignature::KeySignature(const QString& titleMajor, const QString& titleMinor, IconCode::Code icon, Key key)
    : titleMajor(titleMajor), titleMinor(titleMinor), icon(icon), key(key)
{
}

AdditionalInfoModel::KeySignature::KeySignature(const QVariantMap& map)
{
    titleMajor = map[TITLE_MAJOR_KEY].toString();
    titleMinor = map[TITLE_MINOR_KEY].toString();
    icon = static_cast<IconCode::Code>(map[ICON_KEY].toInt());
    key = static_cast<Key>(map[SIGNATURE_KEY].toInt());
}

QVariantMap AdditionalInfoModel::KeySignature::toMap() const
{
    return {
        { TITLE_MAJOR_KEY, titleMajor },
        { TITLE_MINOR_KEY, titleMinor },
        { ICON_KEY, static_cast<int>(icon) },
        { SIGNATURE_KEY, static_cast<int>(key) }
    };
}

AdditionalInfoModel::Tempo::Tempo(const QVariantMap& map)
{
    value = map[VALUE_KEY].toInt();
    noteIcon = static_cast<MusicalSymbolCode>(map[NOTE_ICON_KEY].toInt());
    withDot = map[WITH_DOT_KEY].toBool();
}

QVariantMap AdditionalInfoModel::Tempo::toMap() const
{
    return {
        { VALUE_KEY, value },
        { NOTE_ICON_KEY, static_cast<int>(noteIcon) },
        { WITH_DOT_KEY, withDot },
        { NOTE_SYMBOL_KEY, musicalSymbolToString(noteIcon, withDot) }
    };
}

AdditionalInfoModel::AdditionalInfoModel(QObject* parent)
    : QObject(parent)
{
}

void AdditionalInfoModel::init()
{
    setKeySignature(KeySignature(qtrc("userscore", "None"), qtrc("userscore", "None"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C).toMap());

    setTimeSignatureType(static_cast<int>(TimeSignatureType::Fraction));
    setTimeSignature(FOUR_FOUR_TIME_SIGNATURE);

    setWithTempo(false);

    Tempo defaultTempo;
    defaultTempo.noteIcon = MusicalSymbolCode::CROTCHET;
    defaultTempo.value = 120;
    setTempo(defaultTempo.toMap());

    setWithPickupMeasure(false);
    setMeasureCount(32);
    setPickupTimeSignature(DEFAULT_PICKUP_TIME_SIGNATURE);
}

QVariantList AdditionalInfoModel::keySignatureList() const
{
    QVariantList list = {
        KeySignature(qtrc("userscore", "C major"), qtrc("userscore", "A minor"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C).toMap(),
        KeySignature(qtrc("userscore", "F major"), qtrc("userscore", "D minor"), IconCode::Code::KEY_SIGNATURE_1_FLAT, Key::F).toMap(),
        KeySignature(qtrc("userscore", "Bb major"), qtrc("userscore", "G minor"), IconCode::Code::KEY_SIGNATURE_2_FLAT, Key::B_B).toMap(),
        KeySignature(qtrc("userscore", "Eb major"), qtrc("userscore", "C minor"), IconCode::Code::KEY_SIGNATURE_3_FLAT, Key::E_B).toMap(),
        KeySignature(qtrc("userscore", "Ab major"), qtrc("userscore", "F minor"), IconCode::Code::KEY_SIGNATURE_4_FLAT, Key::A_B).toMap(),
        KeySignature(qtrc("userscore", "Db major"), qtrc("userscore", "Bb minor"), IconCode::Code::KEY_SIGNATURE_5_FLAT, Key::D_B).toMap(),
        KeySignature(qtrc("userscore", "Gb major"), qtrc("userscore", "Eb minor"), IconCode::Code::KEY_SIGNATURE_6_FLAT, Key::G_B).toMap(),
        KeySignature(qtrc("userscore", "Cb major"), qtrc("userscore", "Ab minor"), IconCode::Code::KEY_SIGNATURE_7_FLAT, Key::C_B).toMap(),
        KeySignature(qtrc("userscore", "None"), qtrc("userscore", "None"), IconCode::Code::KEY_SIGNATURE_NONE, Key::C).toMap(),
        KeySignature(qtrc("userscore", "G major"), qtrc("userscore", "E minor"), IconCode::Code::KEY_SIGNATURE_1_SHARP, Key::G).toMap(),
        KeySignature(qtrc("userscore", "D major"), qtrc("userscore", "B minor"), IconCode::Code::KEY_SIGNATURE_2_SHARPS, Key::D).toMap(),
        KeySignature(qtrc("userscore", "A major"), qtrc("userscore", "F# minor"), IconCode::Code::KEY_SIGNATURE_3_SHARPS, Key::A).toMap(),
        KeySignature(qtrc("userscore", "E major"), qtrc("userscore", "C# minor"), IconCode::Code::KEY_SIGNATURE_4_SHARPS, Key::E).toMap(),
        KeySignature(qtrc("userscore", "B major"), qtrc("userscore", "G# minor"), IconCode::Code::KEY_SIGNATURE_5_SHARPS, Key::B).toMap(),
        KeySignature(qtrc("userscore", "F# major"), qtrc("userscore", "D# minor"), IconCode::Code::KEY_SIGNATURE_6_SHARPS,
                     Key::F_S).toMap(),
        KeySignature(qtrc("userscore", "C# major"), qtrc("userscore", "A# minor"), IconCode::Code::KEY_SIGNATURE_7_SHARPS, Key::C_S).toMap()
    };

    return list;
}

QVariantMap AdditionalInfoModel::keySignature() const
{
    return m_keySignature.toMap();
}

QVariantMap AdditionalInfoModel::timeSignature() const
{
    switch (m_timeSignatureType) {
    case TimeSigType::FOUR_FOUR:
        return FOUR_FOUR_TIME_SIGNATURE.toMap();
    case TimeSigType::ALLA_BREVE:
        return ALLA_BREVE_TIME_SIGNATURE.toMap();
    case TimeSigType::CUT_BACH:
        return CUT_BACH_TIME_SIGNATURE.toMap();
        break;
    case TimeSigType::CUT_TRIPLE:
        return CUT_TRIPLE_TIME_SIGNATURE.toMap();
        break;
    case TimeSigType::NORMAL:
        break;
    }

    return m_timeSignature.toMap();
}

int AdditionalInfoModel::timeSignatureType() const
{
    return static_cast<int>(m_timeSignatureType);
}

void AdditionalInfoModel::setTimeSignatureType(int timeSignatureType)
{
    TimeSigType type = static_cast<TimeSigType>(timeSignatureType);
    if (m_timeSignatureType == type) {
        return;
    }

    m_timeSignatureType = type;
    emit timeSignatureChanged();
}

void AdditionalInfoModel::setTimeSignatureNumerator(int numerator)
{
    notation::Fraction newTimeSignature(numerator, m_timeSignature.denominator());
    setTimeSignature(newTimeSignature);
}

void AdditionalInfoModel::setTimeSignatureDenominator(int denominator)
{
    notation::Fraction newTimeSignature(m_timeSignature.numerator(), denominator);
    setTimeSignature(newTimeSignature);
}

void AdditionalInfoModel::setTimeSignature(const notation::Fraction& timeSignature)
{
    if (m_timeSignature == timeSignature) {
        return;
    }

    m_timeSignature = timeSignature;
    emit timeSignatureChanged();
}

QVariantList AdditionalInfoModel::musicSymbolCodes(int number) const
{
    static QMap<QChar, MusicalSymbolCode> numeralsMusicSymbolCodes = {
        { '0', MusicalSymbolCode::ZERO },
        { '1', MusicalSymbolCode::ONE },
        { '2', MusicalSymbolCode::TWO },
        { '3', MusicalSymbolCode::THREE },
        { '4', MusicalSymbolCode::FOUR },
        { '5', MusicalSymbolCode::FIVE },
        { '6', MusicalSymbolCode::SIX },
        { '7', MusicalSymbolCode::SEVEN },
        { '8', MusicalSymbolCode::EIGHT },
        { '9', MusicalSymbolCode::NINE },
    };

    QVariantList result;
    QString numberStr = QString::number(number);
    for (const QChar& numeral: numberStr) {
        result << static_cast<int>(numeralsMusicSymbolCodes[numeral]);
    }

    return result;
}

bool AdditionalInfoModel::withTempo() const
{
    return m_withTempo;
}

QVariantMap AdditionalInfoModel::tempo() const
{
    return m_tempo.toMap();
}

int AdditionalInfoModel::currentTempoNoteIndex() const
{
    QString selectedNoteSymbol = musicalSymbolToString(m_tempo.noteIcon, m_tempo.withDot);
    QVariantList notes = tempoNotes();

    for (int i = 0; i < notes.size(); ++i) {
        QVariantMap note = notes[i].toMap();

        if (note[NOTE_SYMBOL_KEY].toString() == selectedNoteSymbol) {
            return i;
        }
    }

    return -1;
}

QVariantMap AdditionalInfoModel::tempoValueRange() const
{
    return QVariantMap { { MIN_KEY, 20 }, { MAX_KEY, 400 } };
}

QVariantList AdditionalInfoModel::tempoNotes() const
{
    auto makeNote = [](MusicalSymbolCodes::Code icon, bool withDot = false) {
        QVariantMap note;
        note[NOTE_ICON_KEY] = static_cast<int>(icon);
        note[NOTE_SYMBOL_KEY] = musicalSymbolToString(icon, withDot);

        if (withDot) {
            note[WITH_DOT_KEY] = withDot;
        }

        return note;
    };

    constexpr bool WITH_DOT = true;

    QVariantList notes;
    notes << makeNote(MusicalSymbolCode::SEMIQUAVER)
          << makeNote(MusicalSymbolCode::SEMIQUAVER, WITH_DOT)
          << makeNote(MusicalSymbolCode::QUAVER)
          << makeNote(MusicalSymbolCode::QUAVER, WITH_DOT)
          << makeNote(MusicalSymbolCode::CROTCHET)
          << makeNote(MusicalSymbolCode::CROTCHET, WITH_DOT)
          << makeNote(MusicalSymbolCode::MINIM)
          << makeNote(MusicalSymbolCode::MINIM, WITH_DOT)
          << makeNote(MusicalSymbolCode::SEMIBREVE);

    return notes;
}

QVariantMap AdditionalInfoModel::pickupTimeSignature() const
{
    return m_pickupTimeSignature.toMap();
}

void AdditionalInfoModel::setPickupTimeSignatureNumerator(int numerator)
{
    notation::Fraction newTimeSignature(numerator, m_pickupTimeSignature.denominator());
    setPickupTimeSignature(newTimeSignature);
}

void AdditionalInfoModel::setPickupTimeSignatureDenominator(int denominator)
{
    notation::Fraction newTimeSignature(m_pickupTimeSignature.numerator(), denominator);
    setPickupTimeSignature(newTimeSignature);
}

void AdditionalInfoModel::setPickupTimeSignature(const notation::Fraction& pickupTimeSignature)
{
    if (m_pickupTimeSignature == pickupTimeSignature) {
        return;
    }

    m_pickupTimeSignature = pickupTimeSignature;
    emit pickupTimeSignatureChanged();
}

bool AdditionalInfoModel::withPickupMeasure() const
{
    return m_withPickupMeasure;
}

int AdditionalInfoModel::measureCount() const
{
    return m_measureCount;
}

QVariantMap AdditionalInfoModel::measureCountRange() const
{
    return QVariantMap { { MIN_KEY, 1 }, { MAX_KEY, 9999 } };
}

QVariantList AdditionalInfoModel::timeSignatureDenominators() const
{
    return QVariantList { 1, 2, 4, 8, 16, 32, 64 };
}

void AdditionalInfoModel::setKeySignature(QVariantMap keySignature)
{
    if (m_keySignature.toMap() == keySignature) {
        return;
    }

    m_keySignature = KeySignature(keySignature);
    emit keySignatureChanged(keySignature);
}

void AdditionalInfoModel::setTempo(QVariantMap tempo)
{
    if (m_tempo.toMap() == tempo) {
        return;
    }

    m_tempo = Tempo(tempo);
    emit tempoChanged(tempo);
}

void AdditionalInfoModel::setWithTempo(bool withTempo)
{
    if (m_withTempo == withTempo) {
        return;
    }

    m_withTempo = withTempo;
    emit withTempoChanged(m_withTempo);
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
