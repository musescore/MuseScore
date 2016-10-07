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
#include "musescoreCore.h"
#include "style.h"
#include "mscore.h"
#include "sequencer.h"
#include "element.h"
#include "dynamic.h"
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
#include "hairpin.h"
#include "glissando.h"
#include "page.h"
#include "slur.h"
#include "lyrics.h"
#include "accidental.h"
#include "notedot.h"
#include "tie.h"
#include "staff.h"
#include "beam.h"
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
#include "chord.h"
#include "hook.h"
#include "stem.h"
#include "stemslash.h"
#include "fraction.h"
#include "excerpt.h"
#include "spatium.h"
#include "barline.h"

namespace Ms {

bool MScore::debugMode = false;
bool MScore::testMode = false;

// #ifndef NDEBUG
bool MScore::showSegmentShapes   = false;
bool MScore::showMeasureShapes   = false;
bool MScore::noHorizontalStretch = false;
bool MScore::noVerticalStretch   = false;
bool MScore::showBoundingRect    = false;
bool MScore::showCorruptedMeasures = true;
bool MScore::useFallbackFont       = true;
// #endif

bool  MScore::saveTemplateMode = false;
bool  MScore::noGui = false;

MStyle* MScore::_defaultStyle;
MStyle* MScore::_defaultStyleForParts;
MStyle* MScore::_baseStyle;
QString MScore::_globalShare;
int     MScore::_vRaster;
int     MScore::_hRaster;
bool    MScore::_verticalOrientation = false;
qreal   MScore::verticalPageGap = 5.0;
qreal   MScore::horizontalPageGapEven = 1.0;
qreal   MScore::horizontalPageGapOdd = 50.0;

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
int     MScore::division    = 480; // 3840;   // pulses per quarter note (PPQ) // ticks per beat
int     MScore::sampleRate  = 44100;
int     MScore::mtcType;

bool    MScore::noExcerpts = false;
bool    MScore::noImages = false;
bool    MScore::pdfPrinting = false;
double  MScore::pixelRatio  = 0.8;        // DPI / logicalDPI

MPaintDevice* MScore::_paintDevice;

#ifdef SCRIPT_INTERFACE
QQmlEngine* MScore::_qml = 0;
#endif

Sequencer* MScore::seq = 0;
MuseScoreCore* MuseScoreCore::mscoreCore;

extern void initDrumset();
extern void initScoreFonts();
extern QString mscoreGlobalShare;

//---------------------------------------------------------
//   Direction
//---------------------------------------------------------

Direction::Direction(const QString& s)
      {
      if (s == "up")
            val = UP;
      else if (s == "down")
            val = DOWN;
      else if (s == "auto")
            val = AUTO;
      else
            val = s.toInt();
      }

//---------------------------------------------------------
//   Direction::toString
//---------------------------------------------------------

const char* Direction::toString() const
      {
      switch (val) {
            case AUTO: return "auto";
            case UP:   return "up";
            case DOWN: return "down";
            }
      __builtin_unreachable();
      }

//---------------------------------------------------------
//   fillComboBox
//---------------------------------------------------------

void Direction::fillComboBox(QComboBox* cb)
      {
      cb->clear();
      cb->addItem(qApp->translate("Direction", "auto"), int(AUTO));
      cb->addItem(qApp->translate("Direction", "up"),   int(UP));
      cb->addItem(qApp->translate("Direction", "down"), int(DOWN));
      }

static Spatium doubleToSpatium(double d)       { return Spatium(d); }
static TextStyleType intToTextStyleType(int i) { return TextStyleType(i); }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
      {
      if (!QMetaType::registerConverter<Spatium, double>(&Spatium::toDouble))
            qFatal("registerConverter Spatium::toDouble failed");
      if (!QMetaType::registerConverter<double, Spatium>(&doubleToSpatium))
            qFatal("registerConverter douobleToSpatium failed");
      if (!QMetaType::registerConverter<int, TextStyleType>(&intToTextStyleType))
            qFatal("registerConverter intToTextStyleType failed");

#ifdef SCRIPT_INTERFACE
      qRegisterMetaType<Element::Type>     ("ElementType");
      qRegisterMetaType<Note::ValueType>   ("ValueType");

      qRegisterMetaType<Direction::E>("Direction");

      qRegisterMetaType<MScore::DirectionH>("DirectionH");
      qRegisterMetaType<Element::Placement>("Placement");
      qRegisterMetaType<Spanner::Anchor>   ("Anchor");
      qRegisterMetaType<NoteHead::Group>   ("NoteHeadGroup");
      qRegisterMetaType<NoteHead::Type>("NoteHeadType");
      qRegisterMetaType<Segment::Type>("SegmentType");
      qRegisterMetaType<FiguredBassItem::Modifier>("Modifier");
      qRegisterMetaType<FiguredBassItem::Parenthesis>("Parenthesis");
      qRegisterMetaType<FiguredBassItem::ContLine>("ContLine");
      qRegisterMetaType<Volta::Type>("VoltaType");
      qRegisterMetaType<Ottava::Type>("OttavaType");
      qRegisterMetaType<Trill::Type>("TrillType");
      qRegisterMetaType<Dynamic::Range>("DynamicRange");
      qRegisterMetaType<Jump::Type>("JumpType");
      qRegisterMetaType<Marker::Type>("MarkerType");
      qRegisterMetaType<Beam::Mode>("BeamMode");
      qRegisterMetaType<HairpinType>("HairpinType");
      qRegisterMetaType<Lyrics::Syllabic>("Syllabic");
      qRegisterMetaType<LayoutBreak::Type>("LayoutBreakType");
      qRegisterMetaType<Glissando::Type>("GlissandoType");

      //classed enumerations
      qRegisterMetaType<MSQE_TextStyleType::E>("TextStyleType");
      qRegisterMetaType<MSQE_BarLineType::E>("BarLineType");
#endif
      qRegisterMetaType<Fraction>("Fraction");

//      DPMM = DPI / INCH;       // dots/mm

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
      // Try relative path (needed for portable AppImage and non-standard installations)
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../share/" INSTALL_NAME));
      if (dir.exists())
            _globalShare = dir.absolutePath() + "/";
      else // Fall back to default location (e.g. if binary has moved relative to share)
            _globalShare = QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif

      selectColor[0].setNamedColor("#1259d0");   //blue
      selectColor[1].setNamedColor("#009234");   //green
      selectColor[2].setNamedColor("#c04400");   //orange
      selectColor[3].setNamedColor("#70167a");   //purple

      defaultColor        = Qt::black;
      dropColor           = QColor("#1778db");
      defaultPlayDuration = 300;      // ms
      warnPitchRange      = true;
      playRepeats         = true;
      panPlayback         = true;

      lastError           = "";

      layoutBreakColor    = QColor("#5999db");
      frameMarginColor    = QColor("#5999db");
      bgColor.setNamedColor("#dddddd");

      _defaultStyle         = new MStyle();
      Ms::initStyle(_defaultStyle);
      _defaultStyleForParts = 0;
      _baseStyle            = new MStyle(*_defaultStyle);

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
            ":/fonts/FreeSerifItalic.ttf",
            ":/fonts/FreeSerifBoldItalic.ttf",
            ":/fonts/mscoreTab.ttf",
            ":/fonts/mscore-BC.ttf",
            ":/fonts/bravura/BravuraText.otf",
            ":/fonts/gootville/GootvilleText.otf",
            ":/fonts/mscore/MScoreText.ttf",
            };

      for (unsigned i = 0; i < sizeof(fonts)/sizeof(*fonts); ++i) {
            QString s(fonts[i]);
            if (-1 == QFontDatabase::addApplicationFont(s)) {
                  if (!MScore::testMode)
                        qDebug("Mscore: fatal error: cannot load internal font <%s>", qPrintable(s));
                  if (!MScore::debugMode && !MScore::testMode)
                        exit(-1);
                  }
            }
#endif
      initScoreFonts();
      StaffType::initStaffTypes();
      initDrumset();
      FiguredBass::readConfigFile(0);

#ifdef DEBUG_SHAPES
      testShapes();
#endif
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

#ifdef SCRIPT_INTERFACE
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
            qmlRegisterUncreatableType<Direction>("MuseScore", 1, 0, "Direction", tr("You can't create an enumeration"));

            qmlRegisterType<MScore>     ("MuseScore", 1, 0, "MScore");
            qmlRegisterType<MsScoreView>("MuseScore", 1, 0, "ScoreView");
//            qmlRegisterType<QmlPlugin>  ("MuseScore", 1, 0, "MuseScore");
            qmlRegisterType<Score>      ("MuseScore", 1, 0, "Score");
            qmlRegisterType<Segment>    ("MuseScore", 1, 0, "Segment");
            qmlRegisterType<Chord>      ("MuseScore", 1, 0, "Chord");
            qmlRegisterType<Note>       ("MuseScore", 1, 0, "Note");
            qmlRegisterType<NoteHead>   ("MuseScore", 1, 0, "NoteHead");
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
            qmlRegisterType<Hook>       ("MuseScore", 1, 0, "Hook");
            qmlRegisterType<Stem>       ("MuseScore", 1, 0, "Stem");
            qmlRegisterType<StemSlash>  ("MuseScore", 1, 0, "StemSlash");
            qmlRegisterType<Beam>       ("MuseScore", 1, 0, "Beam");
            qmlRegisterType<Excerpt>    ("MuseScore", 1, 0, "Excerpt");
            qmlRegisterType<BarLine>    ("MuseScore", 1, 0, "BarLine");

            qmlRegisterType<FractionWrapper>   ("MuseScore", 1, 1, "Fraction");
            qRegisterMetaType<FractionWrapper*>("FractionWrapper*");

            qmlRegisterUncreatableType<Element>("MuseScore", 1, 0,
               "Element", tr("you cannot create an element"));

            //classed enumerations
            qmlRegisterUncreatableType<MSQE_TextStyleType>("MuseScore", 1, 0, "TextStyleType", tr("You can't create an enum"));
            qmlRegisterUncreatableType<MSQE_BarLineType>("MuseScore", 1, 0, "BarLineType", tr("You can't create an enum"));

            //-----------virtual classes
            qmlRegisterType<ChordRest>();
            qmlRegisterType<SlurTie>();
            qmlRegisterType<Spanner>();
            }
      return _qml;
      }

//---------------------------------------------------------
//   paintDevice
//---------------------------------------------------------

MPaintDevice* MScore::paintDevice()
      {
      if (!_paintDevice)
            _paintDevice = new MPaintDevice();
      return _paintDevice;
      }

int MPaintDevice::metric(PaintDeviceMetric m) const
      {
      switch (m) {
            case QPaintDevice::PdmDpiY:
                  return int(DPI);
            default:
                  printf("debug: metric %d\n", int(m));
                  return 1;
            }
      return 0;
      }

QPaintEngine* MPaintDevice::paintEngine() const
      {
      printf("paint engine\n");
      return 0;
      }

#endif
}

