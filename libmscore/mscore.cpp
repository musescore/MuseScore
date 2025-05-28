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

#include "accidental.h"
#include "beam.h"
#include "chord.h"
#include "config.h"
#include "dynamic.h"
#include "element.h"
#include "figuredbass.h"
#include "fraction.h"
#include "glissando.h"
#include "hairpin.h"
#include "jump.h"
#include "layoutbreak.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "mscore.h"
#include "mscoreview.h"
#include "musescoreCore.h"
#include "note.h"
#include "notedot.h"
#include "ottava.h"
#include "score.h"
#include "sequencer.h"
#include "spanner.h"
#include "spatium.h"
#include "staff.h"
#include "stafftype.h"
#include "style.h"
#include "trill.h"
#include "volta.h"

namespace Ms {

bool MScore::debugMode = false;
bool MScore::testMode = false;

// #ifndef NDEBUG
bool MScore::showSegmentShapes   = false;
bool MScore::showSkylines        = false;
bool MScore::showMeasureShapes   = false;
bool MScore::noHorizontalStretch = false;
bool MScore::noVerticalStretch   = false;
bool MScore::showBoundingRect    = false;
bool MScore::showSystemBoundingRect    = false;
bool MScore::showCorruptedMeasures = true;
bool MScore::useFallbackFont       = true;
// #endif

bool  MScore::saveTemplateMode = false;
bool  MScore::noGui = false;

MStyle* MScore::_defaultStyleForParts;

QString MScore::_globalShare;
int     MScore::_vRaster;
int     MScore::_hRaster;
bool    MScore::_verticalOrientation = false;
qreal   MScore::verticalPageGap = 5.0;
qreal   MScore::horizontalPageGapEven = 1.0;
qreal   MScore::horizontalPageGapOdd = 50.0;

QColor  MScore::selectColor[VOICES];
QColor  MScore::cursorColor;
QColor  MScore::defaultColor;
QColor  MScore::layoutBreakColor;
QColor  MScore::frameMarginColor;
QColor  MScore::bgColor;
QColor  MScore::dropColor;
bool    MScore::warnPitchRange;
bool    MScore::disableMouseEntry;
int     MScore::pedalEventsMinTicks;

bool    MScore::harmonyPlayDisableCompatibility;
bool    MScore::harmonyPlayDisableNew;
bool    MScore::playRepeats;
bool    MScore::panPlayback;
int     MScore::playbackSpeedIncrement;
qreal   MScore::nudgeStep;
qreal   MScore::nudgeStep10;
qreal   MScore::nudgeStep50;
int     MScore::defaultPlayDuration;

QString MScore::lastError;
int     MScore::division    = 480; // 3840;   // pulses per quarter note (PPQ) // ticks per beat
int     MScore::sampleRate  = 44100;
int     MScore::mtcType;

bool    MScore::noExcerpts = false;
bool    MScore::noImages = false;
bool    MScore::pdfPrinting = false;
bool    MScore::svgPrinting = false;

double  MScore::pixelRatio  = 0.8;        // DPI / logicalDPI

MPaintDevice* MScore::_paintDevice;

Sequencer* MScore::seq = 0;
MuseScoreCore* MuseScoreCore::mscoreCore;

extern void initDrumset();
extern QString mscoreGlobalShare;

std::vector<MScoreError> MScore::errorList {
      { MS_NO_ERROR,                     0,    0                                                                           },

      { NO_NOTE_SELECTED,                "s1", QT_TRANSLATE_NOOP("error", "No note selected:\nPlease select a note and retry")                     },
      { NO_CHORD_REST_SELECTED,          "s2", QT_TRANSLATE_NOOP("error", "No chord/rest selected:\nPlease select a chord or rest and retry")      },
      { NO_LYRICS_SELECTED,              "s3", QT_TRANSLATE_NOOP("error", "No note or lyrics selected:\nPlease select a note or lyrics and retry") },
      { NO_NOTE_REST_SELECTED,           "s4", QT_TRANSLATE_NOOP("error", "No note or rest selected:\nPlease select a note or rest and retry")     },
      { NO_FLIPPABLE_SELECTED,           "s5", QT_TRANSLATE_NOOP("error", "No flippable element selected:\nPlease select an element that can be flipped and retry") },
      { NO_STAFF_SELECTED,               "s6", QT_TRANSLATE_NOOP("error", "No staff selected:\nPlease select one or more staves and retry")        },
      { NO_NOTE_FIGUREDBASS_SELECTED,    "s7", QT_TRANSLATE_NOOP("error", "No note or figured bass selected:\nPlease select a note or figured bass and retry") },

      { CANNOT_INSERT_TUPLET,            "t1", QT_TRANSLATE_NOOP("error", "Cannot insert chord/rest in tuplet")                                    },
      { CANNOT_SPLIT_TUPLET,             "t2", QT_TRANSLATE_NOOP("error", "Cannot split tuplet")                                                   },
      { CANNOT_SPLIT_MEASURE_FIRST_BEAT, "m1", QT_TRANSLATE_NOOP("error", "Cannot split measure here:\n" "First beat of measure")                  },
      { CANNOT_SPLIT_MEASURE_TUPLET,     "m2", QT_TRANSLATE_NOOP("error", "Cannot split measure here:\n" "Cannot split tuplet")                    },
      { CANNOT_SPLIT_MEASURE_TOO_SHORT,  "m3", QT_TRANSLATE_NOOP("error", "Cannot split measure here:\n" "Measure would be too short")             },

      { NO_DEST,                         "p1", QT_TRANSLATE_NOOP("error", "No destination to paste")                                               },
      { DEST_TUPLET,                     "p2", QT_TRANSLATE_NOOP("error", "Cannot paste into tuplet")                                              },
      { TUPLET_CROSSES_BAR,              "p3", QT_TRANSLATE_NOOP("error", "Tuplet cannot cross barlines")                                          },
      { DEST_LOCAL_TIME_SIGNATURE,       "p4", QT_TRANSLATE_NOOP("error", "Cannot paste in local time signature")                                  },
      { DEST_TREMOLO,                    "p5", QT_TRANSLATE_NOOP("error", "Cannot paste in tremolo")                                               },
      { NO_MIME,                         "p6", QT_TRANSLATE_NOOP("error", "Nothing to paste")                                                      },
      { DEST_NO_CR,                      "p7", QT_TRANSLATE_NOOP("error", "Destination is not a chord or rest")                                    },
      { CANNOT_CHANGE_LOCAL_TIMESIG_MEASURE_NOT_EMPTY, "l1", QT_TRANSLATE_NOOP("error", "Cannot change local time signature:\nMeasure is not empty") },
      { CANNOT_CHANGE_LOCAL_TIMESIG_HAS_EXCERPTS,      "l1", QT_TRANSLATE_NOOP("error", "Cannot change local time signature:\n"
                                                                                "This score already has part scores. Changing local time "
                                                                                "signatures while part scores are present is not yet supported.")  },
      { CORRUPTED_MEASURE,               "c1", QT_TRANSLATE_NOOP("error", "Cannot change time signature in front of a corrupted measure")          },
      };

MsError MScore::_error { MS_NO_ERROR };

//---------------------------------------------------------
//   Direction
//---------------------------------------------------------

Direction toDirection(const QString& s)
      {
      Direction val;
      if (s == "up")
            val = Direction::UP;
      else if (s == "down")
            val = Direction::DOWN;
      else if (s == "auto")
            val = Direction::AUTO;
      else
            val = Direction(s.toInt());
      return val;
      }

//---------------------------------------------------------
//   Direction::toString
//---------------------------------------------------------

const char* toString(Direction val)
      {
      switch (val) {
            case Direction::AUTO: return "auto";
            case Direction::UP:   return "up";
            case Direction::DOWN: return "down";
            }
      Q_UNREACHABLE();
      }

//---------------------------------------------------------
//   Direction::toUserString
//---------------------------------------------------------

QString toUserString(Direction val)
      {
      switch (val) {
            case Direction::AUTO: return qApp->translate("Direction", "Auto");
            case Direction::UP:   return qApp->translate("Direction", "Up");
            case Direction::DOWN: return qApp->translate("Direction", "Down");
            }
      Q_UNREACHABLE();
      }

//---------------------------------------------------------
//   fillComboBox
//---------------------------------------------------------

void fillComboBoxDirection(QComboBox* cb)
      {
      cb->clear();
      cb->addItem(toUserString(Direction::AUTO), QVariant::fromValue<Direction>(Direction::AUTO));
      cb->addItem(toUserString(Direction::UP),   QVariant::fromValue<Direction>(Direction::UP));
      cb->addItem(toUserString(Direction::DOWN), QVariant::fromValue<Direction>(Direction::DOWN));
      }

//---------------------------------------------------------
//   doubleToSpatium
//---------------------------------------------------------

static Spatium doubleToSpatium(double d)
      {
      return Spatium(d);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
      {
      static bool initDone = false;
      if (initDone)
            return;

      if (!QMetaType::registerConverter<Spatium, double>(&Spatium::toDouble))
            qFatal("registerConverter Spatium::toDouble failed");
      if (!QMetaType::registerConverter<double, Spatium>(&doubleToSpatium))
            qFatal("registerConverter doubleToSpatium failed");
//      if (!QMetaType::registerComparators<Spatium>())
//            qFatal("registerComparators for Spatium failed");

#ifdef SCRIPT_INTERFACE
      qRegisterMetaType<Note::ValueType>   ("ValueType");

      qRegisterMetaType<MScore::DirectionH>("DirectionH");
      qRegisterMetaType<Spanner::Anchor>   ("Anchor");
      qRegisterMetaType<NoteHead::Group>   ("NoteHeadGroup");
      qRegisterMetaType<NoteHead::Type>("NoteHeadType");
      qRegisterMetaType<SegmentType>("SegmentType");
      qRegisterMetaType<FiguredBassItem::Modifier>("Modifier");
      qRegisterMetaType<FiguredBassItem::Parenthesis>("Parenthesis");
      qRegisterMetaType<FiguredBassItem::ContLine>("ContLine");
      qRegisterMetaType<Volta::Type>("VoltaType");
      qRegisterMetaType<OttavaType>("OttavaType");
      qRegisterMetaType<Trill::Type>("TrillType");
      qRegisterMetaType<Dynamic::Range>("DynamicRange");
      qRegisterMetaType<Jump::Type>("JumpType");
      qRegisterMetaType<Marker::Type>("MarkerType");
      qRegisterMetaType<Beam::Mode>("BeamMode");
      qRegisterMetaType<HairpinType>("HairpinType");
      qRegisterMetaType<Lyrics::Syllabic>("Syllabic");
      qRegisterMetaType<LayoutBreak::Type>("LayoutBreakType");

      //classed enumerations
//      qRegisterMetaType<MSQE_StyledPropertyListIdx::E>("StyledPropertyListIdx");
//      qRegisterMetaType<MSQE_BarLineType::E>("BarLineType");
#endif
      qRegisterMetaType<Fraction>("Fraction");

      if (!QMetaType::registerConverter<Fraction, QString>(&Fraction::toString))
          qFatal("registerConverter Fraction::toString failed");

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

      selectColor[0] = QColor(0x0065BF);   //blue
      selectColor[1] = QColor(0x007F00);   //green
      selectColor[2] = QColor(0xC53F00);   //orange
      selectColor[3] = QColor(0xC31989);   //purple

      defaultColor           = Qt::black;
      dropColor              = QColor(0x1778db);
      defaultPlayDuration    = 300;      // ms
      warnPitchRange         = true;
      pedalEventsMinTicks    = 1;
      playRepeats            = true;
      panPlayback            = true;
      playbackSpeedIncrement = 5;

      lastError           = "";

      layoutBreakColor    = QColor(0xA0A0A4);
      frameMarginColor    = QColor(0xA0A0A4);
      bgColor             = QColor(0xdddddd);

      //
      //  initialize styles
      //
      _baseStyle.precomputeValues();
      QSettings s;
      QString defStyle = s.value("score/style/defaultStyleFile").toString();
      if (!(MScore::testMode || defStyle.isEmpty())) {
            QFile f(defStyle);
            if (f.open(QIODevice::ReadOnly)) {
                  qDebug("load default style <%s>", qPrintable(defStyle));
                  _defaultStyle.load(&f);
                  f.close();
                  }
            }
      _defaultStyle.precomputeValues();
      QString partStyle = s.value("score/style/partStyleFile").toString();
      if (!(MScore::testMode || partStyle.isEmpty())) {
            QFile f(partStyle);
            if (f.open(QIODevice::ReadOnly)) {
                  qDebug("load default style for parts <%s>", qPrintable(partStyle));
                  _defaultStyleForParts = new MStyle(_defaultStyle);
                  _defaultStyleForParts->load(&f);
                  _defaultStyleForParts->precomputeValues();
                  }
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
            ":/fonts/musejazz/MuseJazzText.otf",
            ":/fonts/campania/Campania.otf",
            ":/fonts/edwin/Edwin-Roman.otf",
            ":/fonts/edwin/Edwin-Bold.otf",
            ":/fonts/edwin/Edwin-Italic.otf",
            ":/fonts/edwin/Edwin-BdIta.otf",
            ":/fonts/FreeSans.ttf",
            ":/fonts/FreeSerif.ttf",
            ":/fonts/FreeSerifBold.ttf",
            ":/fonts/FreeSerifItalic.ttf",
            ":/fonts/FreeSerifBoldItalic.ttf",
            ":/fonts/mscoreTab.ttf",
            ":/fonts/mscore-BC.ttf",
            ":/fonts/leland/Leland.otf",
            ":/fonts/leland/LelandText.otf",
            ":/fonts/bravura/BravuraText.otf",
            ":/fonts/gootville/GootvilleText.otf",
            ":/fonts/mscore/MScoreText.otf",
            ":/fonts/petaluma/PetalumaText.otf",
            ":/fonts/petaluma/PetalumaScript.otf",
            ":/fonts/finalemaestro/FinaleMaestroText.otf",
            ":/fonts/finalebroadway/FinaleBroadwayText.otf",
            };

      // Include the above internal text fonts into QFontDatabase
      for (unsigned i = 0; i < sizeof(fonts)/sizeof(*fonts); ++i) {
            QString str(fonts[i]);
            if (-1 == QFontDatabase::addApplicationFont(str)) {
                  if (!MScore::testMode)
                        qDebug("Mscore: fatal error: cannot load internal font <%s>", qPrintable(str));
                  if (!MScore::debugMode && !MScore::testMode)
                        exit(-1);
                  }
            }
#endif
// Workaround for QTBUG-73241 (solved in Qt 5.12.2) in Windows 10, see https://musescore.org/en/node/280244
#if defined(Q_OS_WIN) && (QT_VERSION < QT_VERSION_CHECK(5, 12, 2))
if (QOperatingSystemVersion::current().majorVersion() >= 10) {
      const QDir additionalFontsDir(QString("%1/Microsoft/Windows/Fonts").arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)));
      if (additionalFontsDir.exists()) {
            QFileInfoList fileList = additionalFontsDir.entryInfoList();
            for (int i = 0; i < fileList.size(); ++i) {
                  QFileInfo fileInfo = fileList.at(i);
                  QFontDatabase::addApplicationFont(fileInfo.filePath());
                  }
            }
      }
#endif
      ScoreFont::initScoreFonts();
      StaffType::initStaffTypes();
      initDrumset();
      FiguredBass::readConfigFile(0);

#ifdef DEBUG_SHAPES
      testShapes();
#endif

      initDone = true;
      }

//---------------------------------------------------------
//   readDefaultStyle
//---------------------------------------------------------

bool MScore::readDefaultStyle(QString file)
      {
      if (file.isEmpty())
            return false;
      MStyle style = defaultStyle();
      QFile f(file);
      if (!f.open(QIODevice::ReadOnly))
            return false;
      bool rv = style.load(&f, true);
      if (rv)
            setDefaultStyle(style);
      f.close();
      return rv;
      }

//---------------------------------------------------------
//   defaultStyleForPartsHasChanged
//---------------------------------------------------------

void MScore::defaultStyleForPartsHasChanged()
      {
// TODO what is needed here?
//      delete _defaultStyleForParts;
//      _defaultStyleForParts = 0;
      }

//---------------------------------------------------------
//   errorMessage
//---------------------------------------------------------

const char* MScore::errorMessage()
      {
      for (MScoreError& e : errorList) {
            if (e.no == _error)
                  return e.txt;
            }
      return QT_TRANSLATE_NOOP("error", "Unknown error");
      }

//---------------------------------------------------------
//   errorGroup
//---------------------------------------------------------

const char* MScore::errorGroup()
      {
      for (MScoreError& e : errorList) {
            if (e.no == _error)
                  return e.group;
            }
      return "";
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

//---------------------------------------------------------
//   metric
//---------------------------------------------------------

int MPaintDevice::metric(PaintDeviceMetric m) const
      {
      switch (m) {
            case QPaintDevice::PdmDpiY:
                  return int(DPI);
            default:
//printf("debug: metric %d\n", int(m));
                  return 1;
            }
      }

//---------------------------------------------------------
//   paintEngine
//---------------------------------------------------------

QPaintEngine* MPaintDevice::paintEngine() const
      {
//printf("paint engine\n");
      return 0;
      }

}

