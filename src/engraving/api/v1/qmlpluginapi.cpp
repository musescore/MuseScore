/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "qmlpluginapi.h"

#include <QQmlEngine>
#include <QJSValueIterator>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/interval.h"
#include "engraving/types/types.h"

// api
#include "apitypes.h"
#include "engravingapiv1.h"
#include "score.h"
#include "instrument.h"
#include "cursor.h"
#include "elements.h"
#include "selection.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::apiv1;

Enum* PluginAPI::elementTypeEnum = nullptr;
Enum* PluginAPI::accidentalTypeEnum = nullptr;
Enum* PluginAPI::ornamentStyleEnum = nullptr;
Enum* PluginAPI::alignEnum = nullptr;
Enum* PluginAPI::placementEnum = nullptr;
Enum* PluginAPI::placementHEnum = nullptr;
Enum* PluginAPI::textPlaceEnum = nullptr;
Enum* PluginAPI::directionEnum = nullptr;
Enum* PluginAPI::directionHEnum = nullptr;
Enum* PluginAPI::orientationEnum = nullptr;
Enum* PluginAPI::autoOnOffEnum = nullptr;
Enum* PluginAPI::voiceAssignmentEnum = nullptr;
Enum* PluginAPI::spacerTypeEnum = nullptr;
Enum* PluginAPI::layoutBreakTypeEnum = nullptr;
Enum* PluginAPI::durationTypeEnum = nullptr;
Enum* PluginAPI::noteValueTypeEnum = nullptr;
Enum* PluginAPI::beamModeEnum = nullptr;
Enum* PluginAPI::glissandoTypeEnum = nullptr;
Enum* PluginAPI::glissandoStyleEnum = nullptr;
Enum* PluginAPI::harmonyTypeEnum = nullptr;
Enum* PluginAPI::noteHeadTypeEnum = nullptr;
Enum* PluginAPI::noteHeadSchemeEnum = nullptr;
Enum* PluginAPI::noteHeadGroupEnum = nullptr;
Enum* PluginAPI::noteTypeEnum = nullptr;
Enum* PluginAPI::playEventTypeEnum = nullptr;
Enum* PluginAPI::segmentTypeEnum = nullptr;
Enum* PluginAPI::barLineTypeEnum = nullptr;
Enum* PluginAPI::tidEnum = nullptr;
Enum* PluginAPI::lyricsSyllabicEnum = nullptr;
Enum* PluginAPI::spannerAnchorEnum = nullptr;
Enum* PluginAPI::mMRestRangeBracketTypeEnum = nullptr;
Enum* PluginAPI::tupletNumberTypeEnum = nullptr;
Enum* PluginAPI::tupletBracketTypeEnum = nullptr;
Enum* PluginAPI::tripletFeelTypeEnum = nullptr;
Enum* PluginAPI::guitarBendTypeEnum = nullptr;
Enum* PluginAPI::guitarBendShowHoldLineEnum = nullptr;
Enum* PluginAPI::clefTypeEnum = nullptr;
Enum* PluginAPI::clefToBarlinePositionEnum = nullptr;
Enum* PluginAPI::dynamicTypeEnum = nullptr;
Enum* PluginAPI::dynamicSpeedEnum = nullptr;
Enum* PluginAPI::lineTypeEnum = nullptr;
Enum* PluginAPI::hookTypeEnum = nullptr;
Enum* PluginAPI::keyModeEnum = nullptr;
Enum* PluginAPI::arpeggioTypeEnum = nullptr;
Enum* PluginAPI::intervalStepEnum = nullptr;
Enum* PluginAPI::intervalTypeEnum = nullptr;
Enum* PluginAPI::instrumentLabelVisibilityEnum = nullptr;
Enum* PluginAPI::ornamentShowAccidentalEnum = nullptr;
Enum* PluginAPI::partialSpannerDirectionEnum = nullptr;
Enum* PluginAPI::chordStylePresetEnum = nullptr;
Enum* PluginAPI::annotationCategoryEnum = nullptr;
Enum* PluginAPI::playingTechniqueTypeEnum = nullptr;
Enum* PluginAPI::gradualTempoChangeTypeEnum = nullptr;
Enum* PluginAPI::changeMethodEnum = nullptr;
Enum* PluginAPI::changeDirectionEnum = nullptr;
Enum* PluginAPI::accidentalRoleEnum = nullptr;
Enum* PluginAPI::accidentalValEnum = nullptr;
Enum* PluginAPI::fermataTypeEnum = nullptr;
Enum* PluginAPI::chordLineTypeEnum = nullptr;
Enum* PluginAPI::slurStyleTypeEnum = nullptr;
Enum* PluginAPI::tremoloTypeEnum = nullptr;
Enum* PluginAPI::tremoloChordTypeEnum = nullptr;
Enum* PluginAPI::bracketTypeEnum = nullptr;
Enum* PluginAPI::jumpTypeEnum = nullptr;
Enum* PluginAPI::markerTypeEnum = nullptr;
Enum* PluginAPI::staffGroupEnum = nullptr;
Enum* PluginAPI::trillTypeEnum = nullptr;
Enum* PluginAPI::vibratoTypeEnum = nullptr;
Enum* PluginAPI::articulationTextTypeEnum = nullptr;
Enum* PluginAPI::lyricsDashSystemStartEnum = nullptr;
Enum* PluginAPI::noteLineEndPlacementEnum = nullptr;
Enum* PluginAPI::spannerSegmentTypeEnum = nullptr;
Enum* PluginAPI::tiePlacementEnum = nullptr;
Enum* PluginAPI::tieDotsPlacementEnum = nullptr;
Enum* PluginAPI::timeSigPlacementEnum = nullptr;
Enum* PluginAPI::timeSigStyleEnum = nullptr;
Enum* PluginAPI::timeSigVSMarginEnum = nullptr;
Enum* PluginAPI::noteSpellingTypeEnum = nullptr;
Enum* PluginAPI::keyEnum = nullptr;
Enum* PluginAPI::updateModeEnum = nullptr;
Enum* PluginAPI::layoutFlagEnum = nullptr;
Enum* PluginAPI::elementFlagEnum = nullptr;
Enum* PluginAPI::symIdEnum = nullptr;
Enum* PluginAPI::cursorEnum = nullptr;

//---------------------------------------------------------
//   PluginAPI::registerQmlTypes
//---------------------------------------------------------

void PluginAPI::registerQmlTypes()
{
    static bool qmlTypesRegistered = false;
    if (qmlTypesRegistered) {
        return;
    }

    if (-1 == qmlRegisterType<PluginAPI>("MuseScore", 3, 0, "MuseScore")) {
        LOGW("qmlRegisterType failed: MuseScore");
    }

    qmlRegisterUncreatableType<Enum>("MuseScore", 3, 0, "MuseScoreEnum", "Cannot create an enumeration");

    //qmlRegisterType<ScoreView>("MuseScore", 3, 0, "ScoreView");

    qmlRegisterType<Cursor>("MuseScore", 3, 0, "Cursor");
    qmlRegisterAnonymousType<ScoreElement>("MuseScore", 3);
    qmlRegisterAnonymousType<Score>("MuseScore", 3);
    qmlRegisterAnonymousType<EngravingItem>("MuseScore", 3);
    qmlRegisterAnonymousType<Chord>("MuseScore", 3);
    qmlRegisterAnonymousType<Note>("MuseScore", 3);
    qmlRegisterAnonymousType<Segment>("MuseScore", 3);
    qmlRegisterAnonymousType<Measure>("MuseScore", 3);
    qmlRegisterAnonymousType<Part>("MuseScore", 3);
    qmlRegisterAnonymousType<Staff>("MuseScore", 3);
    qmlRegisterAnonymousType<Instrument>("MuseScore", 3);
    qmlRegisterAnonymousType<Channel>("MuseScore", 3);
    qmlRegisterAnonymousType<StringData>("MuseScore", 3);
    qmlRegisterAnonymousType<Excerpt>("MuseScore", 3);
    qmlRegisterAnonymousType<Selection>("MuseScore", 3);
    qmlRegisterAnonymousType<Tie>("MuseScore", 3);
    qmlRegisterType<PlayEvent>("MuseScore", 3, 0, "PlayEvent");

    qmlRegisterAnonymousType<FractionWrapper>("MuseScore", 3);
    qRegisterMetaType<FractionWrapper*>("FractionWrapper*");

    qmlTypesRegistered = true;
}

void PluginAPI::setup(QQmlEngine* e)
{
    // Sync PluginAPI and EngravingApiV1

    QJSValue apiVal = e->globalObject().property("api");
    if (apiVal.isNull()) {
        LOGE() << "not found api object";
        return;
    }

    QJSValue engravingApiVal = apiVal.property("engraving");
    QObject* engravingApiObj = engravingApiVal.toQObject();
    if (!engravingApiObj) {
        LOGE() << "not found api.engraving object";
        return;
    }

    EngravingApiV1* engravingApi = dynamic_cast<EngravingApiV1*>(engravingApiObj);
    if (!engravingApi) {
        LOGE() << "api.engraving object not EngravingApiV1";
        return;
    }

    engravingApi->setApi(this);
}

PluginAPI::PluginAPI(QQuickItem* parent)
    : QQuickItem(parent)
{
    setRequiresScore(true); // by default plugins require a score to work
}

apiv1::Score* PluginAPI::curScore() const
{
    if (currentScore()) {
        return wrap<apiv1::Score>(currentScore(), Ownership::SCORE);
    }

    return nullptr;
}

QQmlListProperty<apiv1::Score> PluginAPI::scores()
{
    NOT_IMPLEMENTED;

    static std::vector<mu::engraving::Score*> scores;

    return wrapContainerProperty<Score>(this, scores);
}

//---------------------------------------------------------
//   writeScore
///   Writes a score to a file.
///   \param s The score which should be saved.
///   \param name Path where to save the score, with or
///   without the filename extension (the extension is
///   determined by \p ext parameter).
///   \param ext Filename extension \b without the dot,
///   e.g. \p "mscz" or \p "pdf". Determines the file
///   format to be used.
//---------------------------------------------------------

bool PluginAPI::writeScore(Score* s, const QString& name, const QString& ext)
{
    if (!s || !s->score()) {
        return false;
    }

    UNUSED(name);
    UNUSED(ext);

    NOT_IMPLEMENTED;
    return false;
}

//---------------------------------------------------------
//   readScore
///   Reads the score from a file and opens it in a new tab
///   \param name Path to the file to be opened.
///   \param noninteractive Can be used to avoid a "save
///   changes" dialog on closing a score that is either
///   imported or was created with an older version of
///   MuseScore.
//---------------------------------------------------------

apiv1::Score* PluginAPI::readScore(const QString& name, bool noninteractive)
{
    UNUSED(name);
    UNUSED(noninteractive);

    NOT_IMPLEMENTED;
    return nullptr;
}

//---------------------------------------------------------
//   closeScore
//---------------------------------------------------------

void PluginAPI::closeScore(apiv1::Score* score)
{
    UNUSED(score);

    NOT_IMPLEMENTED;
}

//---------------------------------------------------------
//   newElement
///   Creates a new element with the given type. The
///   element can be then added to a score via Cursor::add.
///   \param elementType EngravingItem type, should be the value
///   from PluginAPI::PluginAPI::EngravingItem enumeration.
//---------------------------------------------------------

apiv1::EngravingItem* PluginAPI::newElement(int elementType)
{
    mu::engraving::Score* score = currentScore();

    if (!score) {
        return nullptr;
    }

    if (elementType <= int(ElementType::INVALID) || elementType >= int(ElementType::ROOT_ITEM)) {
        LOGW("PluginAPI::newElement: Wrong type ID: %d", elementType);
        return nullptr;
    }

    const ElementType type = ElementType(elementType);
    mu::engraving::EngravingItem* e = Factory::createItem(type, score->dummy());
    return wrap(e, Ownership::PLUGIN);
}

//---------------------------------------------------------
//   removeElement
///   Disposes of an EngravingItem and its children.
///   \param EngravingItem type.
///   \since MuseScore 3.3
//---------------------------------------------------------

void PluginAPI::removeElement(apiv1::EngravingItem* wrapped)
{
    mu::engraving::Score* score = wrapped->element()->score();
    score->deleteItem(wrapped->element());
}

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

apiv1::Score* PluginAPI::newScore(const QString& /*name*/, const QString& part, int measures)
{
    if (currentScore()) {
        currentScore()->endCmd();
    }

    MasterScore* score = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle(iocContext());

    // TODO: Set path/filename
    NOT_IMPLEMENTED << "setting path/filename";

    score->appendPart(Score::instrTemplateFromName(part));
    score->appendMeasures(measures);
    score->doLayout();

    // TODO: Open score
    NOT_IMPLEMENTED << "opening the newly created score";

    qApp->processEvents();
    Q_ASSERT(currentScore() == score);
    score->startCmd(TranslatableString("undoableAction", "New score"));
    return wrap<Score>(score, Ownership::SCORE);
}

void PluginAPI::cmd(const QString& s)
{
    static const QMap<QString, QString> COMPAT_CMD_MAP = {
        { "escape", "notation-escape" },
        { "cut", "notation-cut" },
        { "copy", "notation-copy" },
        { "paste", "notation-paste" },
        { "paste-half", "notation-paste-half" },
        { "paste-double", "notation-paste-double" },
        { "select-all", "notation-select-all" },
        { "delete", "notation-delete" },
        { "next-chord", "notation-move-right" },
        { "prev-chord", "notation-move-left" },
        { "prev-measure", "notation-move-left-quickly" }
    };

    actionsDispatcher()->dispatch(COMPAT_CMD_MAP.value(s, s).toStdString());
}

void PluginAPI::openLog(const QString&)
{
    DEPRECATED;
}

void PluginAPI::closeLog()
{
    DEPRECATED;
}

void PluginAPI::log(const QString& txt)
{
    LOGD() << txt;
}

void PluginAPI::logn(const QString& txt)
{
    LOGD() << txt;
}

void PluginAPI::log2(const QString& txt, const QString& txt2)
{
    LOGD() << txt << txt2;
}

//---------------------------------------------------------
//   newQProcess
///   Not enabled currently (so excluded from plugin docs)
//---------------------------------------------------------

MsProcess* PluginAPI::newQProcess()
{
    NOT_IMPLEMENTED;
    return nullptr;
}

//---------------------------------------------------------
//   PluginAPI::fraction
///  Creates a new fraction with the given numerator and
///  denominator
//---------------------------------------------------------

FractionWrapper* PluginAPI::fraction(int num, int den) const
{
    return wrap(mu::engraving::Fraction(num, den));
}

FractionWrapper* PluginAPI::fractionFromTicks(int ticks) const
{
    return wrap(mu::engraving::Fraction::fromTicks(ticks));
}

void PluginAPI::quit()
{
    emit closeRequested();
    m_closeRequested.notify();
}

mu::engraving::Score* PluginAPI::currentScore() const
{
    if (context()->currentNotation()) {
        return context()->currentNotation()->elements()->msScore();
    }

    return nullptr;
}

QString PluginAPI::pluginType() const
{
    return m_pluginType;
}

void PluginAPI::setPluginType(const QString& newPluginType)
{
    m_pluginType = newPluginType;
}

QString PluginAPI::menuPath() const
{
    return QString();
}

void PluginAPI::setMenuPath(const QString&)
{
    DEPRECATED;
}

QString PluginAPI::title() const
{
    return m_title;
}

void PluginAPI::setTitle(const QString& newTitle)
{
    m_title = newTitle;
}

QString PluginAPI::version() const
{
    return m_version;
}

void PluginAPI::setVersion(const QString& newVersion)
{
    m_version = newVersion;
}

QString PluginAPI::description() const
{
    return m_description;
}

void PluginAPI::setDescription(const QString& newDescription)
{
    m_description = newDescription;
}

QString PluginAPI::dockArea() const
{
    return QString();
}

void PluginAPI::setDockArea(const QString&)
{
    DEPRECATED;
}

bool PluginAPI::requiresScore() const
{
    return m_requiresScore;
}

void PluginAPI::setRequiresScore(bool newRequiresScore)
{
    m_requiresScore = newRequiresScore;
}

QString PluginAPI::thumbnailName() const
{
    return m_thumbnailName;
}

void PluginAPI::setThumbnailName(const QString& newThumbnailName)
{
    m_thumbnailName = newThumbnailName;
}

QString PluginAPI::categoryCode() const
{
    return m_categoryCode;
}

void PluginAPI::setCategoryCode(const QString& newCategoryCode)
{
    m_categoryCode = newCategoryCode;
}

int PluginAPI::division() const
{
    return engraving::Constants::DIVISION;
}

int PluginAPI::mscoreVersion() const
{
    return mscoreMajorVersion() * 10000 + mscoreMinorVersion() * 100 + mscoreUpdateVersion();
}

int PluginAPI::mscoreMajorVersion() const
{
    return application()->version().major();
}

int PluginAPI::mscoreMinorVersion() const
{
    return application()->version().minor();
}

int PluginAPI::mscoreUpdateVersion() const
{
    return application()->version().patch();
}

qreal PluginAPI::mscoreDPI() const
{
    return engraving::DPI;
}

OrnamentIntervalWrapper* PluginAPI::defaultOrnamentInterval() const
{
    return wrap(mu::engraving::DEFAULT_ORNAMENT_INTERVAL);
}

//---------------------------------------------------------
//   PluginAPI::ornamentInterval
///  Creates a new ornament interval with the given step and type
//---------------------------------------------------------

OrnamentIntervalWrapper* PluginAPI::ornamentInterval(int step, int type) const
{
    return wrap(mu::engraving::OrnamentInterval(mu::engraving::IntervalStep(step), mu::engraving::IntervalType(type)));
}

//---------------------------------------------------------
//   PluginAPI::interval
///  Creates a new interval with the given chromatic and diatonic steps
//---------------------------------------------------------

IntervalWrapper* PluginAPI::interval(int diatonic, int chromatic) const
{
    return wrap(mu::engraving::Interval(diatonic, chromatic));
}

//---------------------------------------------------------
//   PluginAPI::intervalFromOrnamentInterval
///  Creates a new interval from a given ornament interval
//---------------------------------------------------------

IntervalWrapper* PluginAPI::intervalFromOrnamentInterval(OrnamentIntervalWrapper* o) const
{
    return wrap(mu::engraving::Interval::fromOrnamentInterval(o->ornamentInterval()));
}
