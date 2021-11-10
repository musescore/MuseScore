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

#include "articulationprofilesrepository.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "log.h"

using namespace mu;
using namespace mu::mpe;
using namespace mu::async;

static const QString SUPPORTED_FAMILIES = "supportedFamilies";
static const QString PATTERNS_KEY = "patterns";
static const QString PATTERN_POS_KEY = "patternPosition";
static const QString OFFSET_POS_KEY = "offsetPosition";
static const QString OFFSET_VAL_KEY = "offsetValue";

// Arrangement
static const QString ARRANGEMENT_PATTERN_KEY = "arrangementPattern";
static const QString DURATION_FACTOR_KEY = "durationFactor";
static const QString TIMESTAMP_OFFSET_KEY = "timestampOffset";

// Pitch
static const QString PITCH_PATTERN_KEY = "pitchPattern";
static const QString PITCH_OFFSETS_KEY = "pitchOffsets";

// Expression
static const QString EXPRESSION_PATTERN = "expressionPattern";
static const QString MAX_AMPLITUDE_LEVEL_KEY = "maxAmplitudeLevel";
static const QString AMPLITUDE_TIME_SHIFT = "amplitudeTimeShift";
static const QString DYNAMIC_OFFSETS_KEY = "dynamicOffsets";

static const std::unordered_map<ArticulationFamily, QString> ARTICULATION_FAMILY_NAMES = {
    { ArticulationFamily::Undefined, "Undefined" },
    { ArticulationFamily::KeyboardsArticulation, "KeyboardsArticulation" },
    { ArticulationFamily::StringsArticulation, "StringsArticulation" },
    { ArticulationFamily::WindsArticulation, "WindsArticulation" },
    { ArticulationFamily::PercussionsArticulation, "PercussionsArticulation" }
};

static const std::unordered_map<ArticulationType, QString> ARTICULATION_TYPE_NAMES = {
    { ArticulationType::Undefined, "Undefined" },
    { ArticulationType::None, "None" },
    { ArticulationType::Staccato, "Staccato" },
    { ArticulationType::Staccatissimo, "Staccatissimo" },
    { ArticulationType::Tenuto, "Tenuto" },
    { ArticulationType::Marcato, "Marcato" },
    { ArticulationType::Accent, "Accent" },
    { ArticulationType::ShortAccent, "ShortAccent" },
    { ArticulationType::VeryShortFermata, "VeryShortFermata" },
    { ArticulationType::ShortFermata, "ShortFermata" },
    { ArticulationType::ShortFermataHenze, "ShortFermataHenze" },
    { ArticulationType::Fermata, "Fermata" },
    { ArticulationType::LongFermata, "LongFermata" },
    { ArticulationType::LongFermataHenze, "LongFermataHenze" },
    { ArticulationType::VeryLongFermata, "VeryLongFermata" },
    { ArticulationType::LaissezVibrer, "LaissezVibrer" },
    { ArticulationType::Subito, "Subito" },
    { ArticulationType::FadeIn, "FadeIn" },
    { ArticulationType::FadeOut, "FadeOut" },
    { ArticulationType::Harmonic, "Harmonic" },
    { ArticulationType::Mute, "Mute" },
    { ArticulationType::Open, "Open" },
    { ArticulationType::Pizzicato, "Pizzicato" },
    { ArticulationType::SnapPizzicato, "SnapPizzicato" },
    { ArticulationType::RandomPizzicato, "RandomPizzicato" },
    { ArticulationType::UpBow, "UpBow" },
    { ArticulationType::DownBow, "DownBow" },
    { ArticulationType::Detache, "Detache" },
    { ArticulationType::Martele, "Martele" },
    { ArticulationType::Jete, "Jete" },
    { ArticulationType::GhostNote, "GhostNote" },
    { ArticulationType::CrossNote, "CrossNote" },
    { ArticulationType::CircleNote, "CircleNote" },
    { ArticulationType::TriangleNote, "TriangleNote" },
    { ArticulationType::DiamondNote, "DiamondNote" },
    { ArticulationType::Fall, "Fall" },
    { ArticulationType::QuickFall, "QuickFall" },
    { ArticulationType::Doit, "Doit" },
    { ArticulationType::Plop, "Plop" },
    { ArticulationType::Scoop, "Scoop" },
    { ArticulationType::Bend, "Bend" },
    { ArticulationType::SlideOutDown, "SlideOutDown" },
    { ArticulationType::SlideOutUp, "SlideOutUp" },
    { ArticulationType::SlideInAbove, "SlideInAbove" },
    { ArticulationType::SlideInBelow, "SlideInBelow" },
    { ArticulationType::Crescendo, "Crescendo" },
    { ArticulationType::Decrescendo, "Decrescendo" },
    { ArticulationType::Glissando, "Glissando" },
    { ArticulationType::Portamento, "Portamento" },
    { ArticulationType::Legato, "Legato" },
    { ArticulationType::Pedal, "Pedal" },
    { ArticulationType::Arpeggio, "Arpeggio" },
    { ArticulationType::ArpeggioUp, "ArpeggioUp" },
    { ArticulationType::ArpeggioDown, "ArpeggioDown" },
    { ArticulationType::ArpeggioBracket, "ArpeggioBracket" },
    { ArticulationType::ArpeggioStraightUp, "ArpeggioStraightUp" },
    { ArticulationType::ArpeggioStraightDown, "ArpeggioStraightDown" },
    { ArticulationType::Vibrato, "Vibrato" },
    { ArticulationType::WideVibrato, "WideVibrato" },
    { ArticulationType::MoltoVibrato, "MoltoVibrato" },
    { ArticulationType::SenzaVibrato, "SenzaVibrato" },
    { ArticulationType::Tremolo8th, "Tremolo8th" },
    { ArticulationType::Tremolo16th, "Tremolo16th" },
    { ArticulationType::Tremolo32nd, "Tremolo32nd" },
    { ArticulationType::Tremolo64th, "Tremolo64th" },
    { ArticulationType::ShortTrill, "ShortTrill" },
    { ArticulationType::Trill, "Trill" },
    { ArticulationType::Mordent, "Mordent" },
    { ArticulationType::PrallMordent, "PrallMordent" },
    { ArticulationType::MordentWithUpperPrefix, "MordentWithUpperPrefix" },
    { ArticulationType::UpMordent, "UpMordent" },
    { ArticulationType::DownMordent, "DownMordent" },
    { ArticulationType::Tremblement, "Tremblement" },
    { ArticulationType::UpPrall, "UpPrall" },
    { ArticulationType::PrallUp, "PrallUp" },
    { ArticulationType::PrallDown, "PrallDown" },
    { ArticulationType::LinePrall, "LinePrall" },
    { ArticulationType::Slide, "Slide" },
    { ArticulationType::Turn, "Turn" },
    { ArticulationType::InvertedTurn, "InvertedTurn" },
    { ArticulationType::TurnWithSlash, "TurnWithSlash" },
    { ArticulationType::Appoggiatura, "Appoggiatura" },
    { ArticulationType::Acciaccatura, "Acciaccatura" },
    { ArticulationType::TremoloBar, "TremoloBar" },
    { ArticulationType::VolumeSwell, "VolumeSwell" }
};

ArticulationsProfilePtr ArticulationProfilesRepository::createNew() const
{
    return std::make_shared<ArticulationsProfile>();
}

ArticulationsProfilePtr ArticulationProfilesRepository::defaultProfile(const ArticulationFamily family) const
{
    NOT_IMPLEMENTED;
    UNUSED(family);
    return nullptr;
}

ArticulationsProfilePtr ArticulationProfilesRepository::loadProfile(const io::path& path) const
{
    RetVal<QByteArray> fileReading = fileSystem()->readFile(path);

    if (!fileReading.ret) {
        LOGE() << "Unable to read profile, path: " << path;
        return nullptr;
    }

    QJsonParseError err;

    QJsonDocument file = QJsonDocument::fromJson(fileReading.val, &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << err.errorString();
        return nullptr;
    }

    ArticulationsProfilePtr result = std::make_shared<ArticulationsProfile>();

    QJsonObject rootObj = file.object();

    result->supportedFamilies = supportedFamiliesFromJson(rootObj.value(SUPPORTED_FAMILIES).toArray());

    QJsonObject articulationPatterns = rootObj.value(PATTERNS_KEY).toObject();

    for (const QString& key : articulationPatterns.keys()) {
        result->updatePatterns(articulationTypeFromString(key),
                               patternsScopeFromJson(articulationPatterns.value(key).toArray()));
    }

    return result;
}

void ArticulationProfilesRepository::saveProfile(const io::path& path, const ArticulationsProfilePtr profilePtr)
{
    IF_ASSERT_FAILED(profilePtr) {
        return;
    }

    QJsonObject rootObj;

    rootObj.insert(SUPPORTED_FAMILIES, supportedFamiliesToJson(profilePtr->supportedFamilies));

    QJsonObject articulationPatterns;

    for (const auto& pair : profilePtr->data()) {
        articulationPatterns.insert(articulationTypeToString(pair.first),
                                    patternsScopeToJson(pair.second));
    }

    rootObj.insert(PATTERNS_KEY, articulationPatterns);

    Ret fileWriting = fileSystem()->writeToFile(path, QJsonDocument(rootObj).toJson());

    if (!fileWriting) {
        LOGE() << "Unable to write MPE Articulation Profile, err: " << fileWriting.toString();
        return;
    }

    m_profileChanged.send(path);
}

Channel<io::path> ArticulationProfilesRepository::profileChanged() const
{
    return m_profileChanged;
}

ArticulationFamily ArticulationProfilesRepository::articulationFamilyFromString(const QString& str) const
{
    auto search = std::find_if(ARTICULATION_FAMILY_NAMES.begin(), ARTICULATION_FAMILY_NAMES.end(), [str](const auto& pair) {
        return pair.second == str;
    });

    if (search == ARTICULATION_FAMILY_NAMES.cend()) {
        return ArticulationFamily::Undefined;
    }

    return search->first;
}

QString ArticulationProfilesRepository::articulationFamilyToString(const ArticulationFamily family) const
{
    auto search = ARTICULATION_FAMILY_NAMES.find(family);

    if (search == ARTICULATION_FAMILY_NAMES.cend()) {
        return QString();
    }

    return search->second;
}

ArticulationType ArticulationProfilesRepository::articulationTypeFromString(const QString& str) const
{
    auto search = std::find_if(ARTICULATION_TYPE_NAMES.begin(), ARTICULATION_TYPE_NAMES.end(), [str](const auto& pair) {
        return pair.second == str;
    });

    if (search == ARTICULATION_TYPE_NAMES.cend()) {
        return ArticulationType::Undefined;
    }

    return search->first;
}

QString ArticulationProfilesRepository::articulationTypeToString(const ArticulationType type) const
{
    auto search = ARTICULATION_TYPE_NAMES.find(type);

    if (search == ARTICULATION_TYPE_NAMES.cend()) {
        return QString();
    }

    return search->second;
}

std::vector<ArticulationFamily> ArticulationProfilesRepository::supportedFamiliesFromJson(const QJsonArray& array) const
{
    std::vector<ArticulationFamily> result;
    result.reserve(array.size());

    for (const QJsonValue& val : array) {
        result.push_back(articulationFamilyFromString(val.toString()));
    }

    return result;
}

QJsonArray ArticulationProfilesRepository::supportedFamiliesToJson(const std::vector<ArticulationFamily>& families) const
{
    QJsonArray result;

    for (const auto& family : families) {
        result.append(articulationFamilyToString(family));
    }

    return result;
}

ArticulationPatternsScope ArticulationProfilesRepository::patternsScopeFromJson(const QJsonArray& array) const
{
    ArticulationPatternsScope result;

    for (const QJsonValue& val : array) {
        QJsonObject patternObj = val.toObject();

        duration_percentage_t position = patternObj.value(PATTERN_POS_KEY).toDouble();

        ArrangementPattern arrangementPattern = arrangementPatternFromJson(patternObj.value(ARRANGEMENT_PATTERN_KEY).toObject());
        PitchPattern pitchPattern = pitchPatternFromJson(patternObj.value(PITCH_PATTERN_KEY).toObject());
        ExpressionPattern expressionPattern = expressionPatternFromJson(patternObj.value(EXPRESSION_PATTERN).toObject());

        ArticulationPattern articulation;
        articulation.arrangementPattern = std::move(arrangementPattern);
        articulation.pitchPattern = std::move(pitchPattern);
        articulation.expressionPattern = std::move(expressionPattern);

        result.emplace(position, std::move(articulation));
    }

    return result;
}

QJsonArray ArticulationProfilesRepository::patternsScopeToJson(const ArticulationPatternsScope& scope) const
{
    QJsonArray result;

    for (const auto& pair : scope) {
        QJsonObject pattern;
        pattern.insert(PATTERN_POS_KEY, pair.first);
        pattern.insert(ARRANGEMENT_PATTERN_KEY, arrangementPatternToJson(pair.second.arrangementPattern));
        pattern.insert(PITCH_PATTERN_KEY, pitchPatternToJson(pair.second.pitchPattern));
        pattern.insert(EXPRESSION_PATTERN, expressionPatternToJson(pair.second.expressionPattern));

        result.append(pattern);
    }

    return result;
}

ArrangementPattern ArticulationProfilesRepository::arrangementPatternFromJson(const QJsonObject& obj) const
{
    ArrangementPattern result;

    result.durationFactor = obj.value(DURATION_FACTOR_KEY).toDouble();
    result.timestampOffset = obj.value(TIMESTAMP_OFFSET_KEY).toDouble();

    return result;
}

QJsonObject ArticulationProfilesRepository::arrangementPatternToJson(const ArrangementPattern& pattern) const
{
    QJsonObject result;

    result.insert(DURATION_FACTOR_KEY, pattern.durationFactor);
    result.insert(TIMESTAMP_OFFSET_KEY, static_cast<int>(pattern.timestampOffset));

    return result;
}

PitchPattern ArticulationProfilesRepository::pitchPatternFromJson(const QJsonObject& obj) const
{
    PitchPattern result;

    QJsonArray offsets = obj.value(PITCH_OFFSETS_KEY).toArray();

    for (const QJsonValue& pitchOffset : offsets) {
        QJsonObject offsetObj = pitchOffset.toObject();

        result.pitchOffsetMap.emplace(offsetObj.value(OFFSET_POS_KEY).toDouble(),
                                      offsetObj.value(OFFSET_VAL_KEY).toDouble());
    }

    return result;
}

QJsonObject ArticulationProfilesRepository::pitchPatternToJson(const PitchPattern& pattern) const
{
    QJsonObject result;

    QJsonArray pitchOffsets;

    for (const auto& pair : pattern.pitchOffsetMap) {
        QJsonObject offsetObj;
        offsetObj.insert(OFFSET_POS_KEY, pair.first);
        offsetObj.insert(OFFSET_VAL_KEY, pair.second);

        pitchOffsets.append(std::move(offsetObj));
    }

    result.insert(PITCH_OFFSETS_KEY, pitchOffsets);

    return result;
}

ExpressionPattern ArticulationProfilesRepository::expressionPatternFromJson(const QJsonObject& obj) const
{
    ExpressionPattern result;

    result.maxAmplitudeLevel = obj.value(MAX_AMPLITUDE_LEVEL_KEY).toDouble();
    result.amplitudeTimeShift = obj.value(AMPLITUDE_TIME_SHIFT).toDouble();

    QJsonArray offsets = obj.value(DYNAMIC_OFFSETS_KEY).toArray();

    for (const QJsonValue& offset : offsets) {
        QJsonObject offsetObj = offset.toObject();
        result.dynamicOffsetMap.emplace(offsetObj.value(OFFSET_POS_KEY).toDouble(),
                                        offsetObj.value(OFFSET_VAL_KEY).toDouble());
    }

    return result;
}

QJsonObject ArticulationProfilesRepository::expressionPatternToJson(const ExpressionPattern& pattern) const
{
    QJsonObject result;

    result.insert(MAX_AMPLITUDE_LEVEL_KEY, pattern.maxAmplitudeLevel);
    result.insert(AMPLITUDE_TIME_SHIFT, pattern.amplitudeTimeShift);

    QJsonArray dynamicOffsets;

    for (const auto& pair : pattern.dynamicOffsetMap) {
        QJsonObject offsetObj;
        offsetObj.insert(OFFSET_POS_KEY, pair.first);
        offsetObj.insert(OFFSET_VAL_KEY, pair.second);

        dynamicOffsets.append(std::move(offsetObj));
    }

    result.insert(DYNAMIC_OFFSETS_KEY, dynamicOffsets);

    return result;
}
