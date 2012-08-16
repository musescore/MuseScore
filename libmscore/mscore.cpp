//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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
#include "figuredbass.h"
#include "stafftype.h"
#include "note.h"

qreal MScore::PDPI = 1200;
qreal MScore::DPI  = 1200;
qreal MScore::DPMM;
bool  MScore::debugMode;

MStyle* MScore::_defaultStyle;
MStyle* MScore::_baseStyle;
QString MScore::_globalShare;
int     MScore::_vRaster;
int     MScore::_hRaster;
QColor  MScore::selectColor[VOICES];
QColor  MScore::defaultColor;
QColor  MScore::layoutBreakColor;
QColor  MScore::bgColor;
QColor  MScore::dropColor;
bool    MScore::warnPitchRange;

bool    MScore::replaceFractions;
bool    MScore::playRepeats;
bool    MScore::panPlayback;
qreal   MScore::nudgeStep;
int     MScore::defaultPlayDuration;
QString MScore::partStyle;
QString MScore::soundFont;
QString MScore::lastError;
bool    MScore::layoutDebug = false;
int     MScore::division    = 480;
int     MScore::sampleRate  = 44100;
int     MScore::mtcType;

Sequencer* MScore::seq = 0;

extern void initSymbols(int);
extern void initStaffTypes();
extern void initDrumset();

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MScore::init()
      {
#ifdef SCRIPT_INTERFACE
      qRegisterMetaType<MScore::ElementType>("ElementType");
      qRegisterMetaType<MScore::ValueType>("ValueType");
      qRegisterMetaType<MScore::Direction>("Direction");
      qRegisterMetaType<MScore::DirectionH>("DirectionH");
      qRegisterMetaType<Note::NoteHeadGroup>("NoteHeadGroup");
      qRegisterMetaType<Note::NoteHeadType>("NoteHeadType");
      qRegisterMetaType<Segment::SegmentType>("SegmentType");
      qRegisterMetaType<FiguredBassItem::Modifier>("Modifier");
      qRegisterMetaType<FiguredBassItem::Parenthesis>("Parenthesis");
#endif

      DPMM = DPI / INCH;       // dots/mm

#ifdef __MINGW32__
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
      _globalShare = dir.absolutePath() + "/";
#else
#ifdef Q_WS_MAC
#ifdef Q_WS_IOS
      {
      extern QString resourcePath();
      _globalShare = resourcePath();
      }
#else
      QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
      _globalShare = dir.absolutePath() + "/";
#endif
#else
      _globalShare = QString( INSTPREFIX "/share/" INSTALL_NAME);
#endif
#endif
      selectColor[0].setRgb(0, 0, 255);     //blue
      selectColor[1].setRgb(0, 150, 0);     //green
      selectColor[2].setRgb(230, 180, 50);  //yellow
      selectColor[3].setRgb(200, 0, 200);   //purple
      defaultColor        = Qt::black;
      dropColor           = Qt::red;
      nudgeStep           = .1;       // in spatium units (default 0.1)
      defaultPlayDuration = 300;      // ms
      warnPitchRange      = true;
      replaceFractions    = true;
      playRepeats         = true;
      panPlayback         = true;
      partStyle           = "";

      lastError           = "";

      layoutBreakColor    = Qt::green;
      soundFont           = _globalShare + "sound/TimGM6mb.sf2";
      bgColor.setRgb(0x76, 0x76, 0x6e);

      _defaultStyle         = new MStyle();
      ::initStyle(_defaultStyle);
      _baseStyle            = new MStyle(*_defaultStyle);

      //
      //  load internal fonts
      //
      //
      // do not load application specific fonts
      // for MAC, they are in Resources/fonts
      //
#if !defined(Q_WS_MAC) && !defined(Q_WS_UIKIT)
      static const char* fonts[] = {
            "mscore-20.otf",
            "mscore1-20.ttf",
            "MuseJazz.ttf",
            "FreeSans.ttf",
            "FreeSerifMscore.ttf",
            "FreeSerifBold.ttf",
            "gonville-20.otf",
//            "mscore_tab_baroque.ttf",
//            "mscore_tab_modern.ttf",
//            "mscore_tab_renaiss.ttf",
//            "mscore_tab_renaiss2.ttf",
            "mscoreTab.ttf",
//            "FiguredBassMHGPL.otf",
            "mscore-BC.ttf"
            };

      for (unsigned i = 0; i < sizeof(fonts)/sizeof(*fonts); ++i) {
            QString s = QString(":/fonts/%1").arg(fonts[i]);
            if (-1 == QFontDatabase::addApplicationFont(s)) {
                  qDebug("Mscore: fatal error: cannot load internal font <%s>\n", fonts[i]);
                  if (!MScore::debugMode)
                        exit(-1);
                  }
            }
#endif
      initSymbols(0);   // init emmentaler symbols
      initStaffTypes();
      initDrumset();
      FiguredBass::readConfigFile(0);
      StaffTypeTablature::readConfigFile(0);
      }

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

MStyle* MScore::defaultStyle()
      {
      return _defaultStyle;
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

