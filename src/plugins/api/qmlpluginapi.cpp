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

#include "qmlpluginapi.h"

#include <QQmlEngine>

#include "engraving/compat/dummyelement.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"

#include "cursor.h"
#include "elements.h"
#include "enums.h"
#include "fraction.h"
#include "instrument.h"
#include "part.h"
#include "score.h"
#include "selection.h"
#include "tie.h"
#include "util.h"

using namespace mu::engraving;

namespace mu::plugins::api {
Enum* PluginAPI::elementTypeEnum = nullptr;
Enum* PluginAPI::accidentalTypeEnum = nullptr;
Enum* PluginAPI::beamModeEnum = nullptr;
Enum* PluginAPI::placementEnum = nullptr;
Enum* PluginAPI::glissandoTypeEnum = nullptr;
Enum* PluginAPI::layoutBreakTypeEnum = nullptr;
Enum* PluginAPI::lyricsSyllabicEnum = nullptr;
Enum* PluginAPI::directionEnum = nullptr;
Enum* PluginAPI::directionHEnum = nullptr;
Enum* PluginAPI::ornamentStyleEnum = nullptr;
Enum* PluginAPI::glissandoStyleEnum = nullptr;
Enum* PluginAPI::tidEnum = nullptr;
Enum* PluginAPI::alignEnum = nullptr;
Enum* PluginAPI::noteTypeEnum = nullptr;
Enum* PluginAPI::playEventTypeEnum = nullptr;
Enum* PluginAPI::noteHeadTypeEnum = nullptr;
Enum* PluginAPI::noteHeadSchemeEnum = nullptr;
Enum* PluginAPI::noteHeadGroupEnum = nullptr;
Enum* PluginAPI::noteValueTypeEnum = nullptr;
Enum* PluginAPI::segmentTypeEnum = nullptr;
Enum* PluginAPI::spannerAnchorEnum = nullptr;
Enum* PluginAPI::symIdEnum = nullptr;
Enum* PluginAPI::harmonyTypeEnum = nullptr;

//---------------------------------------------------------
//   PluginAPI
//---------------------------------------------------------

PluginAPI::PluginAPI(QQuickItem* parent)
    : QmlPlugin(parent)
{
    setRequiresScore(true);                // by default plugins require a score to work
}

//---------------------------------------------------------
//   curScore
//---------------------------------------------------------

Score* PluginAPI::curScore() const
{
    if (currentScore()) {
        return wrap<Score>(currentScore(), Ownership::SCORE);
    }

    return nullptr;
}

//---------------------------------------------------------
//   scores
//---------------------------------------------------------

QQmlListProperty<Score> PluginAPI::scores()
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

Score* PluginAPI::readScore(const QString& name, bool noninteractive)
{
    UNUSED(name);
    UNUSED(noninteractive);

    NOT_IMPLEMENTED;
    return nullptr;
}

//---------------------------------------------------------
//   closeScore
//---------------------------------------------------------

void PluginAPI::closeScore(mu::plugins::api::Score* score)
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

EngravingItem* PluginAPI::newElement(int elementType)
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

void PluginAPI::removeElement(mu::plugins::api::EngravingItem* wrapped)
{
    mu::engraving::Score* score = wrapped->element()->score();
    score->deleteItem(wrapped->element());
}

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

Score* PluginAPI::newScore(const QString& /*name*/, const QString& part, int measures)
{
    if (currentScore()) {
        currentScore()->endCmd();
    }

    MasterScore* score = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle();

    // TODO: Set path/filename
    NOT_IMPLEMENTED << "setting path/filename";

    score->appendPart(Score::instrTemplateFromName(part));
    score->appendMeasures(measures);
    score->doLayout();

    // TODO: Open score
    NOT_IMPLEMENTED << "opening the newly created score";

    qApp->processEvents();
    Q_ASSERT(currentScore() == score);
    score->startCmd();
    return wrap<Score>(score, Ownership::SCORE);
}

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

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

//---------------------------------------------------------
//   openLog
//---------------------------------------------------------

void PluginAPI::openLog(const QString& name)
{
    if (logFile.isOpen()) {
        logFile.close();
    }
    logFile.setFileName(name);
    if (!logFile.open(QIODevice::WriteOnly)) {
        LOGD("PluginAPI::openLog: failed");
    }
}

//---------------------------------------------------------
//   closeLog
//---------------------------------------------------------

void PluginAPI::closeLog()
{
    if (logFile.isOpen()) {
        logFile.close();
    }
}

//---------------------------------------------------------
//   log
//---------------------------------------------------------

void PluginAPI::log(const QString& txt)
{
    if (logFile.isOpen()) {
        logFile.write(txt.toLocal8Bit());
    }
}

//---------------------------------------------------------
//   logn
//---------------------------------------------------------

void PluginAPI::logn(const QString& txt)
{
    log(txt);
    if (logFile.isOpen()) {
        logFile.write("\n");
    }
}

//---------------------------------------------------------
//   log2
//---------------------------------------------------------

void PluginAPI::log2(const QString& txt, const QString& txt2)
{
    logFile.write(txt.toLocal8Bit());
    logFile.write(txt2.toLocal8Bit());
    logFile.write("\n");
}

//---------------------------------------------------------
//   newQProcess
///   Not enabled currently (so excluded from plugin docs)
//---------------------------------------------------------

MsProcess* PluginAPI::newQProcess()
{
    return 0;   // TODO: new MsProcess(this);
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

void PluginAPI::quit()
{
    emit closeRequested();
}

mu::engraving::Score* PluginAPI::currentScore() const
{
    if (context()->currentNotation()) {
        return context()->currentNotation()->elements()->msScore();
    }

    return nullptr;
}

//---------------------------------------------------------
//   PluginAPI::registerQmlTypes
//---------------------------------------------------------

void PluginAPI::registerQmlTypes()
{
    static bool qmlTypesRegistered = false;
    if (qmlTypesRegistered) {
        return;
    }

    qmlRegisterType<MsProcess>("MuseScore", 3, 0, "QProcess");
    qmlRegisterType<FileIO, 1>("FileIO",    3, 0, "FileIO");

    if (-1 == qmlRegisterType<PluginAPI>("MuseScore", 3, 0, "MuseScore")) {
        LOGW("qmlRegisterType failed: MuseScore");
    }

    qmlRegisterUncreatableType<Enum>("MuseScore", 3, 0, "MuseScoreEnum", "Cannot create an enumeration");

    qmlRegisterType<ScoreView>("MuseScore", 3, 0, "ScoreView");

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
} // namespace mu::plugins::api
