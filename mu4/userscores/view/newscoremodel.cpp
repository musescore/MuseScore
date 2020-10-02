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
#include "newscoremodel.h"

#include "log.h"

using namespace mu::userscores;
using namespace mu::actions;
using namespace mu::notation;
using namespace mu::instruments;

NewScoreModel::NewScoreModel(QObject* parent)
    : QObject(parent)
{
}

bool NewScoreModel::createScore(const QVariant& info)
{
    ScoreCreateOptions options = parseOptions(info.toMap());

    auto notation = notationCreator()->newMasterNotation();
    Ret ret = notation->createNew(options);

    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    if (!globalContext()->containsMasterNotation(notation->path())) {
        globalContext()->addMasterNotation(notation);
    }

    globalContext()->setCurrentMasterNotation(notation);

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
    options.tempo = info["tempo"].toDouble();

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
    options.instrumentTemplates = instrumentTesmplates(info["instrumentIds"].toList());

    return options;
}

QList<InstrumentTemplate> NewScoreModel::instrumentTesmplates(const QVariantList& templateIds) const
{
    if (templateIds.isEmpty()) {
        return QList<InstrumentTemplate>();
    }

    QList<InstrumentTemplate> result;
    InstrumentTemplateHash templates = instrumensRepository()->instrumentsMeta().val.instrumentTemplates;
    for (const QVariant& templateIdVar: templateIds) {
        QString templateId = templateIdVar.toString();
        if (!templates.contains(templateId)) {
            LOGW() << "Template not found" << templateId;
            continue;
        }

        result << templates[templateId];
    }

    return result;
}
