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
#include "instrument.h"
#include "score.h"
#include "part.h"
#include "util.h"
#include "selection.h"
#include "tie.h"

#include "shortcut.h"
#include "musescore.h"
#include "libmscore/musescoreCore.h"

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
      if (!s || !s->score())
            return false;
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
///   Creates a new element with the given type. The
///   element can be then added to a score via Cursor::add.
///   \param elementType Element type, should be the value
///   from PluginAPI::PluginAPI::Element enumeration.
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
      if (msc()->currentScore())
            msc()->currentScore()->endCmd();
      MasterScore* score = new MasterScore(MScore::defaultStyle());
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
      Shortcut* sc = Shortcut::getShortcut(qPrintable(s));
      if (sc)
            msc()->cmd(sc->action());
      else
            qDebug("PluginAPI:cmd: not found <%s>", qPrintable(s));
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
///   Not enabled currently (so excluded from plugin docs)
//---------------------------------------------------------

MsProcess* PluginAPI::newQProcess()
      {
      return 0; // TODO: new MsProcess(this);
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
      if (qmlTypesRegistered)
            return;

      const char* enumErr = "You can't create an enumeration";
      qmlRegisterType<MsProcess>  ("MuseScore", 3, 0, "QProcess");
      qmlRegisterType<FileIO, 1>  ("FileIO",    3, 0, "FileIO");
      //-----------mscore bindings
      qmlRegisterUncreatableMetaObject(Ms::staticMetaObject, "MuseScore", 3, 0, "Ms", enumErr);
//            qmlRegisterUncreatableType<Direction>("MuseScore", 3, 0, "Direction", QObject::tr(enumErr));

      if (-1 == qmlRegisterType<PluginAPI>  ("MuseScore", 3, 0, "MuseScore"))
            qWarning("qmlRegisterType failed: MuseScore");

      qmlRegisterUncreatableType<Enum>("MuseScore", 3, 0, "MuseScoreEnum", "Cannot create an enumeration");

//             qmlRegisterType<MScore>     ("MuseScore", 3, 0, "MScore");
      qmlRegisterType<ScoreView>("MuseScore", 3, 0, "ScoreView");

      qmlRegisterType<Cursor>("MuseScore", 3, 0, "Cursor");
      qmlRegisterType<ScoreElement>();
      qmlRegisterType<Score>();
      qmlRegisterType<Element>();
      qmlRegisterType<Chord>();
      qmlRegisterType<Note>();
      qmlRegisterType<Segment>();
      qmlRegisterType<Measure>();
      qmlRegisterType<Part>();
      qmlRegisterType<Instrument>();
      qmlRegisterType<Channel>();
      qmlRegisterType<StringData>();
      qmlRegisterType<Excerpt>();
      qmlRegisterType<Selection>();
      qmlRegisterType<Tie>();
      qmlRegisterType<PlayEvent>("MuseScore", 3, 0, "PlayEvent");
      //qmlRegisterType<Hook>();
      //qmlRegisterType<Stem>();
      //qmlRegisterType<StemSlash>();
      //qmlRegisterType<Beam>();

#if 0
      qmlRegisterType<NoteHead>   ("MuseScore", 1, 0, "NoteHead");
      qmlRegisterType<Accidental> ("MuseScore", 1, 0, "Accidental");
      qmlRegisterType<Rest>       ("MuseScore", 1, 0, "Rest");
      qmlRegisterType<StaffText>  ("MuseScore", 1, 0, "StaffText");
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
