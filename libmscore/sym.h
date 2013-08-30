//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYM_H__
#define __SYM_H__

#include "config.h"

class QPainter;

namespace Ms {

class TextStyle;

extern void initSymbols(int);
extern int symIdx2fontId(int symIdx);
extern QFont fontId2font(int id);

//---------------------------------------------------------
//   SymId
//---------------------------------------------------------

enum SymId {
      noSym = -1,
      clefEightSym = 0,
      clefOneSym,
      clefFiveSym,

      wholerestSym,
      halfrestSym,
      outsidewholerestSym,
      outsidehalfrestSym,
      longarestSym,
      breverestSym,
      rest4Sym,
      rest8Sym,
      clasquartrestSym,
      rest16Sym,
      rest32Sym,
      rest64Sym,
      rest128Sym,
      rest_M3,

      varcodaSym,
      brackettipsRightUp,
      brackettipsRightDown,
      brackettipsLeftUp,
      brackettipsLeftDown,

      zeroSym,
      oneSym,
      twoSym,
      threeSym,
      fourSym,
      fiveSym,
      sixSym,
      sevenSym,
      eightSym,
      nineSym,

      sharpSym,
      sharpArrowUpSym,
      sharpArrowDownSym,
      sharpArrowBothSym,
      sharpslashSym,
      sharpslash2Sym,
      sharpslash3Sym,
      sharpslash4Sym,

      naturalSym,
      naturalArrowUpSym,
      naturalArrowDownSym,
      naturalArrowBothSym,
      flatSym,
      flatArrowUpSym,
      flatArrowDownSym,
      flatArrowBothSym,

      flatslashSym,
      flatslash2Sym,
      mirroredflat2Sym,
      mirroredflatSym,
      mirroredflatslashSym,
      flatflatSym,
      flatflatslashSym,
      sharpsharpSym,
      soriSym,
      koronSym,

      rightparenSym,
      leftparenSym,
      dotSym,

      longaupSym,
      longadownSym,
      brevisheadSym,
      brevisdoubleheadSym,
      wholeheadSym,
      halfheadSym,
      quartheadSym,
      wholediamondheadSym,
      halfdiamondheadSym,
      diamondheadSym,
      s0triangleHeadSym,
      d1triangleHeadSym,
      u1triangleHeadSym,
      u2triangleHeadSym,
      d2triangleHeadSym,
      wholeslashheadSym,
      halfslashheadSym,
      quartslashheadSym,
      wholecrossedheadSym,
      halfcrossedheadSym,
      crossedheadSym,
      xcircledheadSym,
      s0doHeadSym,
      d1doHeadSym,
      u1doHeadSym,
      d2doHeadSym,
      u2doHeadSym,
      s0reHeadSym,
      u1reHeadSym,
      d1reHeadSym,
      u2reHeadSym,
      d2reHeadSym,
      s0miHeadSym,
      s1miHeadSym,
      s2miHeadSym,
      u0faHeadSym,
      d0faHeadSym,
      u1faHeadSym,
      d1faHeadSym,
      u2faHeadSym,
      d2faHeadSym,
      s0laHeadSym,
      s1laHeadSym,
      s2laHeadSym,
      s0tiHeadSym,
      u1tiHeadSym,
      d1tiHeadSym,
      u2tiHeadSym,
      d2tiHeadSym,

      s0solHeadSym,
      s1solHeadSym,
      s2solHeadSym,

      open01arrowHeadSym,
      open0M1arrowHeadSym,
      open11arrowHeadSym,
      open1M1arrowHeadSym,
      close01arrowHeadSym,
      close0M1arrowHeadSym,
      close11arrowHeadSym,
      close1M1arrowHeadSym,

      ufermataSym,
      dfermataSym,

      snappizzicatoSym,
      thumbSym,
      ushortfermataSym,
      dshortfermataSym,
      ulongfermataSym,
      dlongfermataSym,
      uverylongfermataSym,
      dverylongfermataSym,

      sforzatoaccentSym,
      esprSym,
      staccatoSym,
      ustaccatissimoSym,
      dstaccatissimoSym,
      tenutoSym,
      uportatoSym,
      dportatoSym,
      umarcatoSym,
      dmarcatoSym,
      ouvertSym,
      halfopenSym,
      plusstopSym,
      upbowSym,
      downbowSym,
      reverseturnSym,
      turnSym,
      trillSym,
      upedalheelSym,
      dpedalheelSym,

      upedaltoeSym,
      dpedaltoeSym,
      flageoletSym,
      segnoSym,
      varsegnoSym,
      codaSym,

      rcommaSym,
      lcommaSym,
      lvarcommaSym,
      rvarcommaSym,

      arpeggioSym,
      trillelementSym,
      arpeggioarrowdownSym,
      arpeggioarrowupSym,
      trilelementSym,
      prallSym,
      mordentSym,
      prallprallSym,
      prallmordentSym,
      upprallSym,

      downprallSym,
      upmordentSym,
      downmordentSym,
      lineprallSym,
      pralldownSym,
      prallupSym,
      schleiferSym,

      caesuraCurvedSym,
      caesuraStraight,

      eighthflagSym,
      sixteenthflagSym,
      thirtysecondflagSym,
      sixtyfourthflagSym,
      flag128Sym,
      deighthflagSym,
      gracedashSym,
      dgracedashSym,
      dsixteenthflagSym,
      dthirtysecondflagSym,
      dsixtyfourthflagSym,
      dflag128Sym,
      altoclefSym,
      caltoclefSym,
      bassclefSym,
      cbassclefSym,
      trebleclefSym,
      ctrebleclefSym,
      percussionclefSym,
      cpercussionclefSym,
      tabclefSym,
      ctabclefSym,
      fourfourmeterSym,
      allabreveSym,
      pedalasteriskSym,
      pedaldashSym,
      pedaldotSym,
      pedalPSym,
      pedaldSym,
      pedaleSym,
      pedalPedSym,
      accDiscantSym,
      accDotSym,
      accFreebaseSym,
      accStdbaseSym,
      accBayanbaseSym,
      accOldEESym,
      accpushSym,
      accpullSym,

      letterfSym,
      lettermSym,
      letterpSym,
      letterrSym,
      lettersSym,
      letterzSym,

      letterTSym,
      letterSSym,
      letterPSym,

      plusSym,
      commaSym,
      hyphenSym,
      periodSym,
      spaceSym,

      longaupaltSym,
      longadownaltSym,
      brevisheadaltSym,

      timesigcdotSym,
      timesigoSym,
      timesigocutSym,
      timesigodotSym,

      tabclef2Sym,

      octave8,
      octave8va,
      octave8vb,
      octave15,
      octave15ma,
      octave15mb,
      octave22,
      octave22ma,
      octave22mb,
      lastSym
      };

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

class Sym {
      int _code;
      int fontId;
      qreal w;
      QRectF _bbox;
      QPointF _attach;

#ifdef USE_GLYPHS
      QGlyphRun glyphs;       // cached values
      void genGlyphs(const QRawFont&);
#endif

      static QVector<const char*> symNames;
      static QVector<QString> symUserNames;
      static QHash<QString, SymId> lnhash;

   public:
      Sym() { _code = -1; }
      Sym(int c, int fid, qreal x=0.0, qreal y=0.0);
      Sym(int c, int fid, const QPointF&, const QRectF&);

      QFont font() const                   { return fontId2font(fontId); }
      const QRectF bbox(qreal mag) const;
      qreal height(qreal mag) const        { return _bbox.height() * mag; }
      qreal width(qreal mag) const         { return w * mag;  }
      QPointF attach(qreal mag) const      { return _attach * mag;   }
      int code() const                     { return _code;    }
      int getFontId() const                { return fontId;   }
      int setFontId(int v)                 { return fontId = v;   }
      void draw(QPainter* painter, qreal mag) const;
      void draw(QPainter* painter, qreal mag, const QPointF& pos) const;
      void draw(QPainter* painter, qreal mag, const QPointF& pos, int n) const;
      void setAttach(const QPointF& r)     { _attach = r; }
      bool isValid() const                 { return _code != 0; }
      QRectF getBbox() const               { return _bbox; }
      QPointF getAttach() const            { return _attach; }
      QString toString() const;

      static SymId name2id(const QString& s) { return lnhash.value(s, noSym); }     // return noSym if not found
      static const char* id2name(SymId id);
      static QString id2userName(SymId id)   { return symUserNames[id]; }
      static SymId userName2id(const QString& s)      { return (SymId)(symUserNames.indexOf(s)); }
      static void init();
      };

extern QVector<Sym> symbols[2];

extern QString symToHtml(const Sym&, int leftMargin=0, const TextStyle* ts = 0, qreal sp=10.0);
extern QString symToHtml(const Sym&, const Sym&, int leftMargin=0);
extern QRawFont fontId2RawFont(int fontId);

}     // namespace Ms
#endif

