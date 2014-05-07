//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#include "style.h"
#include "mscore.h"
#include "sequencer.h"
#include "accidental.h"
#include "figuredbass.h"
#include "stafftype.h"
#include "note.h"
#include "spanner.h"
#include "volta.h"
#include "ottava.h"
#include "trill.h"
#include "repeat.h"
#include "jump.h"
#include "marker.h"
#include "layoutbreak.h"
#include "page.h"
#include "slur.h"
#include "lyrics.h"
#include "accidental.h"
#include "notedot.h"
#include "tie.h"
#include "staff.h"
#include "timesig.h"
#include "part.h"
#include "measure.h"
#include "score.h"
#include "keysig.h"
#include "harmony.h"
#include "cursor.h"
#include "stafftext.h"
#include "mscoreview.h"
#include "plugins.h"
// #include "mscore/qmlplugin.h"

namespace Ms {

qreal MScore::PDPI = 1200;
qreal MScore::DPI  = 1200;
qreal MScore::DPMM;
bool  MScore::debugMode;
bool  MScore::testMode = false;
bool  MScore::noGui = false;

MStyle* MScore::_defaultStyle;
MStyle* MScore::_defaultStyleForParts;
MStyle* MScore::_baseStyle;
QString MScore::_globalShare;
int     MScore::_vRaster;
int     MScore::_hRaster;
QColor  MScore::selectColor[VOICES];
QColor  MScore::defaultColor;
QColor  MScore::layoutBreakColor;
QColor  MScore::frameMarginColor;
QColor  MScore::bgColor;
QColor  MScore::dropColor;
bool    MScore::warnPitchRange;

bool    MScore::playRepeats;
bool    MScore::panPlayback;
qreal   MScore::nudgeStep;
qreal   MScore::nudgeStep10;
qreal   MScore::nudgeStep50;
int     MScore::defaultPlayDuration;
// QString MScore::partStyle;
QString MScore::lastError;
bool    MScore::layoutDebug = false;
int     MScore::division    = 480;   // pulses per quarter note (PPQ) // ticks per beat
int     MScore::sampleRate  = 44100;
int     MScore::mtcType;

bool    MScore::noExcerpts = false;
bool    MScore::noImages = false;

#ifdef SCRIPT_INTERFACE
QQmlEngine* MScore::_qml = 0;
#endif

Sequencer* MScore::seq = 0;

extern void initDrumset();
extern void initScoreFonts();
extern QString mscoreGlobalShare;

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
      {
      qDebug("================= sizeof element %d", sizeof(Element));
#ifdef SCRIPT_INTERFACE
      qRegisterMetaType<Element::ElementType>("ElementType");
      qRegisterMetaType<MScore::ValueType>("ValueType");
      qRegisterMetaType<MScore::Direction>("Direction");
      qRegisterMetaType<MScore::DirectionH>("DirectionH");
      qRegisterMetaType<Element::Placement>("Placement");
      qRegisterMetaType<Accidental::AccidentalRole>("AccidentalRole");
      qRegisterMetaType<Accidental::AccidentalType>("AccidentalType");
      qRegisterMetaType<Spanner::Anchor>("Anchor");
      qRegisterMetaType<NoteHeadGroup>("NoteHeadGroup");
      qRegisterMetaType<NoteHeadType>("NoteHeadType");
      qRegisterMetaType<Segment::SegmentType>("SegmentType");
      qRegisterMetaType<FiguredBassItem::Modifier>("Modifier");
      qRegisterMetaType<FiguredBassItem::Parenthesis>("Parenthesis");
      qRegisterMetaType<VoltaType>("VoltaType");
      qRegisterMetaType<OttavaType>("OttavaType");
      qRegisterMetaType<Trill::TrillType>("TrillType");
      qRegisterMetaType<Element::DynamicRange>("DynamicRange");
      qRegisterMetaType<JumpType>("JumpType");
      qRegisterMetaType<MarkerType>("MarkerType");
      qRegisterMetaType<BeamMode>("BeamMode");
      qRegisterMetaType<LayoutBreak::LayoutBreakType>("LayoutBreakType");
//      qRegisterMetaType<TextStyle>("TextStyle");
#endif

      DPMM = DPI / INCH;       // dots/mm

#ifdef Q_OS_WIN
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
      _globalShare = dir.absolutePath() + "/";
#elif defined(Q_OS_IOS)
      {
      extern QString resourcePath();
      _globalShare = resourcePath();
      }

#elif defined(Q_OS_MAC)
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
      _globalShare = dir.absolutePath() + "/";
#else
      _globalShare = QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif

      selectColor[0].setNamedColor("#2456aa");   //blue
      selectColor[1].setNamedColor("#1a8239");   //green
      selectColor[2].setNamedColor("#d79112");   //yellow
      selectColor[3].setNamedColor("#75112b");   //purple

      defaultColor        = Qt::black;
      dropColor           = Qt::red;
      defaultPlayDuration = 300;      // ms
      warnPitchRange      = true;
      playRepeats         = true;
      panPlayback         = true;

      lastError           = "";

      layoutBreakColor    = QColor("#6abed3");
      frameMarginColor    = QColor("#6abed3");
      bgColor.setNamedColor("#dddddd");

      _defaultStyle         = new MStyle();
      Ms::initStyle(_defaultStyle);
      _defaultStyleForParts = 0;
      _baseStyle            = new MStyle(*_defaultStyle);
      void setPageFormat(const PageFormat& pf);

      if (!MScore::testMode) {
            // QPrinter::PaperSize ps = QPrinter().paperSize();      // get default paper size
            QSizeF psf = QPrinter().paperSize(QPrinter::Inch);
            PaperSize ps("system", psf.width(), psf.height());
            PageFormat pf;
            pf.setSize(&ps);
            _defaultStyle->setPageFormat(pf);
            }

      //
      //  load internal fonts
      //
      //
      // do not load application specific fonts
      // for MAC, they are in Resources/fonts
      //
#if !defined(Q_OS_MAC) && !defined(Q_OS_IOS)
      static const char* fonts[] = {
            ":/fonts/MuseJazz.ttf",
            ":/fonts/FreeSans.ttf",
            ":/fonts/FreeSerif.ttf",
            ":/fonts/FreeSerifBold.ttf",
            ":/fonts/mscoreTab.ttf",
            ":/fonts/mscore-BC.ttf",
            };

      for (unsigned i = 0; i < sizeof(fonts)/sizeof(*fonts); ++i) {
            QString s(fonts[i]);
            if (-1 == QFontDatabase::addApplicationFont(s)) {
                  qDebug("Mscore: fatal error: cannot load internal font <%s>", qPrintable(s));
                  if (!MScore::debugMode)
                        exit(-1);
                  }
            }
#endif
      initScoreFonts();
      StaffType::initStaffTypes();
      initDrumset();
      FiguredBass::readConfigFile(0);
      }

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

MStyle* MScore::defaultStyle()
      {
      return _defaultStyle;
      }

//---------------------------------------------------------
//   defaultStyleForParts
//---------------------------------------------------------

MStyle* MScore::defaultStyleForParts()
      {
      if (!_defaultStyleForParts) {
            QSettings s;
            QString partStyle = s.value("partStyle").toString();
            if (!partStyle.isEmpty()) {
                  QFile f(partStyle);
                  if (f.open(QIODevice::ReadOnly)) {
                        MStyle* s = new MStyle(*defaultStyle());
                        if (s->load(&f))
                              _defaultStyleForParts = s;
                        else
                              delete s;
                        }
                  }
            }
      return _defaultStyleForParts;
      }

//---------------------------------------------------------
//   baseStyle
//---------------------------------------------------------

MStyle* MScore::baseStyle()
      {
      return _baseStyle;
      }

//---------------------------------------------------------
//   setDefaultStyle
//---------------------------------------------------------

void MScore::setDefaultStyle(MStyle* s)
      {
      delete _defaultStyle;
      _defaultStyle = s;
      }

//---------------------------------------------------------
//   defaultStyleForPartsHasChanged
//---------------------------------------------------------

void MScore::defaultStyleForPartsHasChanged()
      {
      delete _defaultStyleForParts;
      _defaultStyleForParts = 0;
      }

//---------------------------------------------------------
//   qml
//---------------------------------------------------------

QQmlEngine* MScore::qml()
      {
      if (_qml == 0) {
            //-----------some qt bindings
            _qml = new QQmlEngine;
#ifdef Q_OS_WIN
            QStringList importPaths;
            QDir dir(QCoreApplication::applicationDirPath() + QString("/../qml"));
            importPaths.append(dir.absolutePath());
            _qml->setImportPathList(importPaths);
#endif
#ifdef Q_OS_MAC
            QStringList importPaths;
            QDir dir(mscoreGlobalShare + QString("/qml"));
            importPaths.append(dir.absolutePath());
            _qml->setImportPathList(importPaths);
#endif
            qmlRegisterType<MsProcess>  ("MuseScore", 1, 0, "QProcess");
            qmlRegisterType<FileIO, 1>  ("FileIO",    1, 0, "FileIO");
            //-----------mscore bindings
            qmlRegisterType<MScore>     ("MuseScore", 1, 0, "MScore");
            qmlRegisterType<MsScoreView>("MuseScore", 1, 0, "ScoreView");
//            qmlRegisterType<QmlPlugin>  ("MuseScore", 1, 0, "MuseScore");
            qmlRegisterType<Score>      ("MuseScore", 1, 0, "Score");
            qmlRegisterType<Segment>    ("MuseScore", 1, 0, "Segment");
            qmlRegisterType<Chord>      ("MuseScore", 1, 0, "Chord");
            qmlRegisterType<Note>       ("MuseScore", 1, 0, "Note");
            qmlRegisterType<Accidental> ("MuseScore", 1, 0, "Accidental");
            qmlRegisterType<Rest>       ("MuseScore", 1, 0, "Rest");
            qmlRegisterType<Measure>    ("MuseScore", 1, 0, "Measure");
            qmlRegisterType<Cursor>     ("MuseScore", 1, 0, "Cursor");
            qmlRegisterType<StaffText>  ("MuseScore", 1, 0, "StaffText");
            qmlRegisterType<Part>       ("MuseScore", 1, 0, "Part");
            qmlRegisterType<Staff>      ("MuseScore", 1, 0, "Staff");
            qmlRegisterType<Harmony>    ("MuseScore", 1, 0, "Harmony");
            qmlRegisterType<PageFormat> ("MuseScore", 1, 0, "PageFormat");
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

            qmlRegisterUncreatableType<Element>("MuseScore", 1, 0,
               "Element", tr("you cannot create an element"));

            //-----------virtual classes
            qmlRegisterType<ChordRest>();
            qmlRegisterType<SlurTie>();
            qmlRegisterType<Spanner>();
            }
      return _qml;
      }
}

