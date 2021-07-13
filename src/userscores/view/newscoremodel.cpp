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
#include "newscoremodel.h"

#include "log.h"

#include "ui/view/musicalsymbolcodes.h"

using namespace mu::userscores;
using namespace mu::notation;
using namespace mu::ui;

using PreferredScoreCreationMode = IUserScoresConfiguration::PreferredScoreCreationMode;

NewScoreModel::NewScoreModel(QObject* parent)
    : QObject(parent)
{
}

QString NewScoreModel::preferredScoreCreationMode() const
{
    switch (configuration()->preferredScoreCreationMode()) {
    case PreferredScoreCreationMode::FromInstruments: return "FromInstruments";
    case PreferredScoreCreationMode::FromTemplate: return "FromTemplate";
    }

    return "";
}

bool NewScoreModel::createScore(const QVariant& info)
{
    ScoreCreateOptions options = parseOptions(info.toMap());

    auto project = notationCreator()->newNotationProject();
    Ret ret = project->createNew(options);

    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    if (!globalContext()->containsNotationProject(project->path())) {
        globalContext()->addNotationProject(project);
    }

    globalContext()->setCurrentNotationProject(project);

    bool isScoreCreatedFromInstruments = options.templatePath.empty();
    updatePreferredScoreCreationMode(isScoreCreatedFromInstruments);

    return true;
}

ScoreCreateOptions NewScoreModel::parseOptions(const QVariantMap& info) const
{
    ScoreCreateOptions options;

    options.title = info["title"].toString();
    options.subtitle = info["subtitle"].toString();
    options.composer = info["composer"].toString();
    options.lyricist = info["lyricist"].toString();
    options.copyright = info["copyright"].toString();

    options.withTempo = info["withTempo"].toBool();

    QVariantMap tempo = info["tempo"].toMap();
    options.tempo.valueBpm = tempo["value"].toInt();
    options.tempo.duration = noteIconToDurationType(tempo["noteIcon"].toInt());
    options.tempo.withDot = tempo["withDot"].toBool();

    QVariantMap timeSignature = info["timeSignature"].toMap();
    options.timesigType = static_cast<TimeSigType>(info["timeSignatureType"].toInt());
    options.timesigNumerator = timeSignature["numerator"].toInt();
    options.timesigDenominator = timeSignature["denominator"].toInt();

    QVariantMap keySignature = info["keySignature"].toMap();
    options.key = static_cast<Key>(keySignature["key"].toInt());
    options.keyMode = static_cast<KeyMode>(keySignature["mode"].toInt());

    QVariantMap measuresPickup = info["pickupTimeSignature"].toMap();
    options.withPickupMeasure = info["withPickupMeasure"].toBool();
    options.measures = info["measureCount"].toInt();
    options.measureTimesigNumerator = measuresPickup["numerator"].toInt();
    options.measureTimesigDenominator = measuresPickup["denominator"].toInt();

    options.templatePath = info["templatePath"].toString();

    QVariantMap partMap = info["parts"].toMap();
    for (const QVariant& obj: partMap["instruments"].toList()) {
        QVariantMap objMap = obj.toMap();
        Q_ASSERT(!objMap["isExistingPart"].toBool());

        PartInstrument pi;

        pi.isExistingPart = false;
        pi.isSoloist = false;
        pi.partId = QString();
        pi.instrument = objMap["instrument"].value<Instrument>();

        options.parts << pi;
    }

    options.order = info["scoreOrder"].value<ScoreOrder>();

    return options;
}

DurationType NewScoreModel::noteIconToDurationType(int noteIconCode) const
{
    static const QMap<MusicalSymbolCodes::Code, DurationType> iconToDuration {
        { MusicalSymbolCodes::Code::SEMIBREVE, DurationType::V_WHOLE },
        { MusicalSymbolCodes::Code::MINIM, DurationType::V_HALF },
        { MusicalSymbolCodes::Code::CROTCHET, DurationType::V_QUARTER },
        { MusicalSymbolCodes::Code::QUAVER, DurationType::V_EIGHTH },
        { MusicalSymbolCodes::Code::SEMIQUAVER, DurationType::V_16TH }
    };

    MusicalSymbolCodes::Code symbol = static_cast<MusicalSymbolCodes::Code>(noteIconCode);
    return iconToDuration.value(symbol, DurationType::V_QUARTER);
}

void NewScoreModel::updatePreferredScoreCreationMode(bool isScoreCreatedFromInstruments)
{
    if (isScoreCreatedFromInstruments) {
        configuration()->setPreferredScoreCreationMode(PreferredScoreCreationMode::FromInstruments);
    } else {
        configuration()->setPreferredScoreCreationMode(PreferredScoreCreationMode::FromTemplate);
    }
}
