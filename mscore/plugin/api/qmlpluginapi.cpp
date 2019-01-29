//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "qmlpluginapi.h"
#include "cursor.h"
#include "elements.h"
#include "fraction.h"
#include "score.h"
#ifndef TESTROOT
#include "shortcut.h"
#endif
#include "libmscore/musescoreCore.h"
#include "libmscore/score.h"
#include "libmscore/plugins.h" // TODO: remove

#include <QQmlEngine>

namespace Ms {
namespace PluginAPI {

Enum* const PluginAPI::elementTypeEnum = wrapEnum<Ms::ElementType>();
Enum* const PluginAPI::accidentalTypeEnum = wrapEnum<Ms::AccidentalType>();
Enum* const PluginAPI::beamModeEnum = wrapEnum<Ms::Beam::Mode>();
Enum* const PluginAPI::placementEnum = wrapEnum<Ms::Placement>();
Enum* const PluginAPI::glissandoTypeEnum = wrapEnum<Ms::GlissandoType>();
Enum* const PluginAPI::layoutBreakTypeEnum = wrapEnum<Ms::LayoutBreak::Type>();
Enum* const PluginAPI::lyricsSyllabicEnum = wrapEnum<Ms::Lyrics::Syllabic>();
Enum* const PluginAPI::directionEnum = wrapEnum<Ms::Direction>();
Enum* const PluginAPI::directionHEnum = wrapEnum<Ms::MScore::DirectionH>();
Enum* const PluginAPI::ornamentStyleEnum = wrapEnum<Ms::MScore::OrnamentStyle>();
Enum* const PluginAPI::glissandoStyleEnum = wrapEnum<Ms::GlissandoStyle>();
Enum* const PluginAPI::tidEnum = wrapEnum<Ms::Tid>();
Enum* const PluginAPI::noteHeadTypeEnum = wrapEnum<Ms::NoteHead::Type>();
Enum* const PluginAPI::noteHeadGroupEnum = wrapEnum<Ms::NoteHead::Group>();
Enum* const PluginAPI::noteValueTypeEnum = wrapEnum<Ms::Note::ValueType>();
Enum* const PluginAPI::segmentTypeEnum = wrapEnum<Ms::SegmentType>();
Enum* const PluginAPI::spannerAnchorEnum = wrapEnum<Ms::Spanner::Anchor>();

//---------------------------------------------------------
//   PluginAPI
//---------------------------------------------------------

PluginAPI::PluginAPI(QQuickItem* parent)
   : Ms::QmlPlugin(parent)
      {
      setRequiresScore(true);              // by default plugins require a score to work
      }

//---------------------------------------------------------
//   curScore
//---------------------------------------------------------

Score* PluginAPI::curScore() const
      {
      return wrap<Score>(msc()->currentScore(), Ownership::SCORE);
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
//---------------------------------------------------------

bool PluginAPI::writeScore(Score* s, const QString& name, const QString& ext)
      {
      if (!s || !s->score())
            return false;
      return msc()->saveAs(s->score(), true, name, ext);
      }

//---------------------------------------------------------
//   readScore
//
// noninteractive can be used to avoid a 'save changes'
// dialog on closing a score that is either imported
// or was created with an older version of MuseScore
//---------------------------------------------------------

Score* PluginAPI::readScore(const QString& name, bool noninteractive)
      {
      Ms::Score* score = msc()->openScore(name, true);
      if (score) {
            if (noninteractive)
                  score->setCreated(false);
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
//---------------------------------------------------------

Element* PluginAPI::newElement(int elementType)
      {
      Ms::Score* score = msc()->currentScore();
      if (!score)
            return nullptr;
      if (elementType <= int(ElementType::INVALID) || elementType >= int(ElementType::MAXTYPE)) {
            qWarning("PluginAPI::newElement: Wrong type ID: %d", elementType);
            return nullptr;
            }
      const ElementType type = ElementType(elementType);
      Ms::Element* e = Ms::Element::create(type, score);
      return wrap(e, Ownership::PLUGIN);
      }

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

Score* PluginAPI::newScore(const QString& name, const QString& part, int measures)
      {
      if (msc()->currentScore())
            msc()->currentScore()->endCmd();
      MasterScore* score = new MasterScore(MScore::defaultStyle());
      score->setName(name);
      score->appendPart(part);
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
#ifdef TESTROOT
      // TODO: testing this function requires including
      // shortcuts system to mtest testutils library
      // as well as some way to execute these commands
      // without MuseScore instance.
      Q_UNUSED(s);
      qFatal("PluginAPI::cmd is not testable currently");
#else
      Shortcut* sc = Shortcut::getShortcut(qPrintable(s));
      if (sc)
            msc()->cmd(sc->action());
      else
            qDebug("PluginAPI:cmd: not found <%s>", qPrintable(s));
#endif
      }

//---------------------------------------------------------
//   openLog
//---------------------------------------------------------

void PluginAPI::openLog(const QString& name)
      {
      if (logFile.isOpen())
            logFile.close();
      logFile.setFileName(name);
      if (!logFile.open(QIODevice::WriteOnly))
            qDebug("PluginAPI::openLog: failed");
      }

//---------------------------------------------------------
//   closeLog
//---------------------------------------------------------

void PluginAPI::closeLog()
      {
      if (logFile.isOpen())
            logFile.close();
      }

//---------------------------------------------------------
//   log
//---------------------------------------------------------

void PluginAPI::log(const QString& txt)
      {
      if (logFile.isOpen())
            logFile.write(txt.toLocal8Bit());
      }

//---------------------------------------------------------
//   logn
//---------------------------------------------------------

void PluginAPI::logn(const QString& txt)
      {
      log(txt);
      if (logFile.isOpen())
            logFile.write("\n");
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
//---------------------------------------------------------

MsProcess* PluginAPI::newQProcess()
      {
      return 0; // TODO: new MsProcess(this);
      }

//---------------------------------------------------------
//   PluginAPI::fraction
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
      if (qmlTypesRegistered)
            return;

      const char* enumErr = "You can't create an enumeration";
//TODO-ws            qmlRegisterType<MsProcess>  ("MuseScore", 3, 0, "QProcess");
      qmlRegisterType<FileIO, 1>  ("FileIO",    3, 0, "FileIO");
      //-----------mscore bindings
      qmlRegisterUncreatableMetaObject(Ms::staticMetaObject, "MuseScore", 3, 0, "Ms", enumErr);
//            qmlRegisterUncreatableType<Direction>("MuseScore", 3, 0, "Direction", QObject::tr(enumErr));

      if (-1 == qmlRegisterType<PluginAPI>  ("MuseScore", 3, 0, "MuseScore"))
            qWarning("qmlRegisterType failed: MuseScore");

//             qmlRegisterType<MScore>     ("MuseScore", 3, 0, "MScore");
//TODO-ws            qmlRegisterType<MsScoreView>("MuseScore", 3, 0, "ScoreView");

      qmlRegisterType<Cursor>("MuseScore", 3, 0, "Cursor");
      qmlRegisterType<ScoreElement>();
      qmlRegisterType<Score>();
      qmlRegisterType<Element>();
      qmlRegisterType<Chord>();
      qmlRegisterType<Note>();
      qmlRegisterType<Segment>();
      qmlRegisterType<Measure>();
#if 0
      qmlRegisterType<NoteHead>   ("MuseScore", 1, 0, "NoteHead");
      qmlRegisterType<Accidental> ("MuseScore", 1, 0, "Accidental");
      qmlRegisterType<Rest>       ("MuseScore", 1, 0, "Rest");
      qmlRegisterType<StaffText>  ("MuseScore", 1, 0, "StaffText");
      qmlRegisterType<Part>       ("MuseScore", 1, 0, "Part");
      qmlRegisterType<Staff>      ("MuseScore", 1, 0, "Staff");
      qmlRegisterType<Harmony>    ("MuseScore", 1, 0, "Harmony");
      qmlRegisterType<TimeSig>    ("MuseScore", 1, 0, "TimeSig");
      qmlRegisterType<KeySig>     ("MuseScore", 1, 0, "KeySig");
      qmlRegisterType<Slur>       ("MuseScore", 1, 0, "Slur");
      qmlRegisterType<Tie>        ("MuseScore", 1, 0, "Tie");
      qmlRegisterType<NoteDot>    ("MuseScore", 1, 0, "NoteDot");
      qmlRegisterType<FiguredBass>("MuseScore", 1, 0, "FiguredBass");
      qmlRegisterType<Text>       ("MuseScore", 1, 0, "MText");
      qmlRegisterType<Lyrics>     ("MuseScore", 1, 0, "Lyrics");
      qmlRegisterType<FiguredBassItem>("MuseScore", 1, 0, "FiguredBassItem");
      qmlRegisterType<LayoutBreak>("MuseScore", 1, 0, "LayoutBreak");
      qmlRegisterType<Hook>       ("MuseScore", 1, 0, "Hook");
      qmlRegisterType<Stem>       ("MuseScore", 1, 0, "Stem");
      qmlRegisterType<StemSlash>  ("MuseScore", 1, 0, "StemSlash");
      qmlRegisterType<Beam>       ("MuseScore", 1, 0, "Beam");
      qmlRegisterType<Excerpt>    ("MuseScore", 1, 0, "Excerpt");
      qmlRegisterType<BarLine>    ("MuseScore", 1, 0, "BarLine");


      //classed enumerations
      qmlRegisterUncreatableType<MSQE_StyledPropertyListIdx>("MuseScore", 1, 0, "StyledPropertyListIdx", QObject::tr("You can't create an enum"));
      qmlRegisterUncreatableType<MSQE_BarLineType>("MuseScore", 1, 0, "BarLineType", enumErr);

      //-----------virtual classes
      qmlRegisterType<ChordRest>();
      qmlRegisterType<SlurTie>();
      qmlRegisterType<Spanner>();
#endif
      qmlRegisterType<FractionWrapper>();
      qRegisterMetaType<FractionWrapper*>("FractionWrapper*");

      qmlTypesRegistered = true;
      }

}
}
