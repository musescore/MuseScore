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
#include "cursor.h"
#include "elements.h"
#include "fraction.h"
#include "instrument.h"
#include "score.h"
#include "part.h"
#include "util.h"
#include "selection.h"
#include "tie.h"

#include "libmscore/masterscore.h"
#include "libmscore/musescoreCore.h"
#include "engraving/compat/scoreaccess.h"

#include <QQmlEngine>

namespace Ms {
namespace PluginAPI {
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
    : Ms::QmlPlugin(parent)
{
    setRequiresScore(true);                // by default plugins require a score to work
}

//---------------------------------------------------------
//   curScore
//---------------------------------------------------------

Score* PluginAPI::curScore() const
{
    if (msc()->currentScore()) {
        return wrap<Score>(msc()->currentScore(), Ownership::SCORE);
    }

    return nullptr;
}

//---------------------------------------------------------
//   scores
//---------------------------------------------------------

QQmlListProperty<Score> PluginAPI::scores()
{
    return wrapContainerProperty<Score>(this, msc()->scores());
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
    return msc()->saveAs(s->score(), true, name, ext);
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
    Ms::Score* score = msc()->openScore(name, !noninteractive);
    if (score) {
        if (noninteractive) {
            score->setCreated(false);
        }
    }
    return wrap<Score>(score, Ownership::SCORE);
}

//---------------------------------------------------------
//   closeScore
//---------------------------------------------------------

void PluginAPI::closeScore(Ms::PluginAPI::Score* score)
{
    msc()->closeScore(score->score());
}

//---------------------------------------------------------
//   newElement
///   Creates a new element with the given type. The
///   element can be then added to a score via Cursor::add.
///   \param elementType Element type, should be the value
///   from PluginAPI::PluginAPI::Element enumeration.
//---------------------------------------------------------

Element* PluginAPI::newElement(int elementType)
{
    Ms::Score* score = msc()->currentScore();
    if (!score) {
        return nullptr;
    }
    if (elementType <= int(ElementType::INVALID) || elementType >= int(ElementType::MAXTYPE)) {
        qWarning("PluginAPI::newElement: Wrong type ID: %d", elementType);
        return nullptr;
    }
    const ElementType type = ElementType(elementType);
    Ms::Element* e = Ms::Element::create(type, score);
    return wrap(e, Ownership::PLUGIN);
}

//---------------------------------------------------------
//   removeElement
///   Disposes of an Element and its children.
///   \param Element type.
///   \since MuseScore 3.3
//---------------------------------------------------------

void PluginAPI::removeElement(Ms::PluginAPI::Element* wrapped)
{
    Ms::Score* score = wrapped->element()->score();
    score->deleteItem(wrapped->element());
}

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

Score* PluginAPI::newScore(const QString& name, const QString& part, int measures)
{
    if (msc()->currentScore()) {
        msc()->currentScore()->endCmd();
    }
    MasterScore* score = mu::engraving::compat::ScoreAccess::createMasterScoreWithDefaultStyle();
    score->setName(name);
    score->appendPart(Score::instrTemplateFromName(part));
    score->appendMeasures(measures);
    score->doLayout();
    const int view = msc()->appendScore(score);
    msc()->setCurrentView(0, view);
    qApp->processEvents();
    Q_ASSERT(msc()->currentScore() == score);
    score->startCmd();
    return wrap<Score>(score, Ownership::SCORE);
}

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void PluginAPI::cmd(const QString& s)
{
    shortcuts()->activate(s.toStdString());
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
        qDebug("PluginAPI::openLog: failed");
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
    return wrap(Ms::Fraction(num, den));
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

    const char* enumErr = "You can't create an enumeration";
    qmlRegisterType<MsProcess>("MuseScore", 3, 0, "QProcess");
    qmlRegisterType<FileIO, 1>("FileIO",    3, 0, "FileIO");
    //-----------mscore bindings
    qmlRegisterUncreatableMetaObject(Ms::staticMetaObject, "MuseScore", 3, 0, "Ms", enumErr);

    if (-1 == qmlRegisterType<PluginAPI>("MuseScore", 3, 0, "MuseScore")) {
        qWarning("qmlRegisterType failed: MuseScore");
    }

    qmlRegisterUncreatableType<Enum>("MuseScore", 3, 0, "MuseScoreEnum", "Cannot create an enumeration");

    qmlRegisterType<ScoreView>("MuseScore", 3, 0, "ScoreView");

    qmlRegisterType<Cursor>("MuseScore", 3, 0, "Cursor");
    qmlRegisterAnonymousType<ScoreElement>("MuseScore", 3);
    qmlRegisterAnonymousType<Score>("MuseScore", 3);
    qmlRegisterAnonymousType<Element>("MuseScore", 3);
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

MuseScoreCore* PluginAPI::msc() const
{
    static MuseScoreCore mscStatic;
    if (this->context() && this->context()->currentNotation()) {
        mscStatic.setCurrentScore(this->context()->currentNotation()->elements()->msScore());
    } else {
        mscStatic.setCurrentScore(0);
    }
    return &mscStatic;
}
}
}
