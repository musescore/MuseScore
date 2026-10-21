/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "global/translation.h"
#include "log.h"

#include "ui/view/musicalsymbolcodes.h"

#include "project/internal/instrumentnamestranslator.h"
#include "types/projectcreateoptions.h"

using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;

using PreferredScoreCreationMode = IProjectConfiguration::PreferredScoreCreationMode;

NewScoreModel::NewScoreModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
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

QVariantList NewScoreModel::instrumentNamesLanguages() const
{
    return InstrumentNamesTranslator().languageChoices(configuration()->recentInstrumentNamesLanguages());
}

QString NewScoreModel::lastInstrumentNamesLanguage() const
{
    return configuration()->lastInstrumentNamesLanguage();
}

QString NewScoreModel::instrumentNamesExample(const QString& languageCode) const
{
    //! An instrument whose name shows both the language and the names of the keys
    static const muse::String EXAMPLE_INSTRUMENT_ID = u"bb-clarinet";

    const mu::engraving::InstrumentTemplate& templ = instrumentsRepository()->instrumentTemplate(EXAMPLE_INSTRUMENT_ID);
    if (!templ.isValid()) {
        return QString();
    }

    InstrumentNamesTranslator translator;
    translator.load(languageCode.isEmpty() ? translator.interfaceLanguageCode() : languageCode);

    //: An example of the names of instruments in the chosen language, e.g. "Example: Clarinet in B♭"
    return muse::qtrc("project/newscore", "Example: %1").arg(translator.exampleName(templ).toQString());
}

bool NewScoreModel::createScore(const QVariant& info)
{
    ProjectCreateOptions options = parseOptions(info.toMap());

    auto project = notationCreator()->newProject(iocContext());
    Ret ret = project->createNew(options);

    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    globalContext()->setCurrentProject(project);

    bool isScoreCreatedFromInstruments = options.templatePath.empty();
    updatePreferredScoreCreationMode(isScoreCreatedFromInstruments);

    configuration()->setLastInstrumentNamesLanguage(options.instrumentNamesLanguage);
    configuration()->setRecentInstrumentNamesLanguages(
        InstrumentNamesTranslator::addRecentLanguage(configuration()->recentInstrumentNamesLanguages(), options.instrumentNamesLanguage));

    return true;
}

ProjectCreateOptions NewScoreModel::parseOptions(const QVariantMap& info) const
{
    ProjectCreateOptions projectOptions;

    projectOptions.title = info["title"].toString();
    projectOptions.subtitle = info["subtitle"].toString();
    projectOptions.composer = info["composer"].toString();
    projectOptions.lyricist = info["lyricist"].toString();
    projectOptions.copyright = info["copyright"].toString();

    projectOptions.templatePath = info["templatePath"].toString();
    projectOptions.instrumentNamesLanguage = info["instrumentNamesLanguage"].toString();

    ScoreCreateOptions& scoreOptions = projectOptions.scoreOptions;

    scoreOptions.withTempo = info["withTempo"].toBool();

    QVariantMap tempo = info["tempo"].toMap();
    scoreOptions.tempo.valueBpm = tempo["value"].toInt();
    scoreOptions.tempo.duration = noteIconToDurationType(tempo["noteIcon"].toInt());
    scoreOptions.tempo.withDot = tempo["withDot"].toBool();

    QVariantMap timeSignature = info["timeSignature"].toMap();
    scoreOptions.timesigType = static_cast<TimeSigType>(info["timeSignatureType"].toInt());

    const int timesigNumerator = timeSignature["numerator"].toInt();
    const int timesigDenominator = timeSignature["denominator"].toInt();
    scoreOptions.globalTimesig = Fraction(timesigNumerator, timesigDenominator);

    QVariantMap keySignature = info["keySignature"].toMap();
    scoreOptions.key = static_cast<Key>(keySignature["key"].toInt());

    QVariantMap measuresPickup = info["pickupTimeSignature"].toMap();
    scoreOptions.totalMeasures = info["measureCount"].toInt();

    scoreOptions.withPickupMeasure = info["withPickupMeasure"].toBool();
    const int pickupNumerator = measuresPickup["numerator"].toInt();
    const int pickupDenominator = measuresPickup["denominator"].toInt();
    scoreOptions.pickupTimesig = Fraction(pickupNumerator, pickupDenominator);

    QVariantList instruments = info["instruments"].toList();

    for (const QVariant& obj : instruments) {
        QVariantMap objMap = obj.toMap();

        PartInstrument pi;

        String instrumentId = objMap["instrumentId"].toString();
        pi.instrumentTemplate = instrumentsRepository()->instrumentTemplate(instrumentId);
        pi.isExistingPart = objMap["isExistingPart"].toBool();
        pi.isSoloist = objMap["isSoloist"].toBool();

        scoreOptions.parts << pi;
    }

    QVariantMap orderMap = info["scoreOrder"].toMap();
    scoreOptions.order = instrumentsRepository()->order(orderMap["id"].toString());
    scoreOptions.order.customized = orderMap["customized"].toBool();

    return projectOptions;
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
