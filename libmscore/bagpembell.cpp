//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "xml.h"
#include "bagpembell.h"

namespace Ms {

// Embellishment names and note sequences
      
BagpipeEmbellishmentInfo BagpipeEmbellishment::BagpipeEmbellishmentList[] = {

      // Single Grace notes
      // These don't look really nice due to varying stem length
      { "Single grace low G", "LG" },
      { "Single grace low A", "LA" },
      { "Single grace B", "B" },
      { "Single grace C", "C" },
      { "Single grace D", "D" },
      { "Single grace E", "E" },
      { "Single grace F", "F" },
      { "Single grace high G", "HG" },
      { "Single grace high A", "HA" },
      
      // Double Grace notes
      { "Double grace", "D LG" },
      { "Double grace", "D LA" },
      { "Double grace", "D B" },
      { "Double grace", "D C" },
      { "Double grace", "E LG" },
      { "Double grace", "E LA" },
      { "Double grace", "E B" },
      { "Double grace", "E C" },
      { "Double grace", "E D" },
      
      { "Double grace", "F LG" },
      { "Double grace", "F LA" },
      { "Double grace", "F B" },
      { "Double grace", "F C" },
      { "Double grace", "F D" },
      { "Double grace", "F E" },
      
      { "Double grace", "HG LG" },
      { "Double grace", "HG LA" },
      { "Double grace", "HG B" },
      { "Double grace", "HG C" },
      { "Double grace", "HG D" },
      { "Double grace", "HG E" },
      { "Double grace", "HG F" },
      
      { "Double grace", "HA LG" },
      { "Double grace", "HA LA" },
      { "Double grace", "HA B" },
      { "Double grace", "HA C" },
      { "Double grace", "HA D" },
      { "Double grace", "HA E" },
      { "Double grace", "HA F" },
      { "Double grace", "HA HG" },
      
      // Half Doublings
      { "Half doubling on low G", "LG D" },
      { "Half doubling on low A", "LA D" },
      { "Half doubling on B", "B D" },
      { "Half doubling on C", "C D" },
      { "Half doubling on D", "D E" },
      { "Half doubling on E", "E F" },
      { "Half doubling on F", "F HG" },
      // ? { "Half doubling on high G", "HG F" },
      // ? { "Half doubling on high A", "HA HG" },

      // Regular Doublings
      { "Doubling on high G", "HG F" },
      { "Doubling on high A", "HA HG" },
      
      // Half Strikes
      { "Half strike on low A", "LA LG" },
      { "Half strike on B", "B LG" },
      { "Half strike on C", "C LG" },
      { "Half strike on D", "D LG" },
      { "Half strike on D", "D C" },
      { "Half strike on E", "E LA" },
      { "Half strike on F", "F E" },
      { "Half strike on high G", "HG F" },
      
      // Regular Grip
      { "Grip", "D LG" },

      // D Throw
      { "Half D throw", "D C" },

      // Regular Doublings (continued)
      { "Doubling on low G",  "HG LG D" },
      { "Doubling on low A",  "HG LA D" },
      { "Doubling on B",      "HG B D" },
      { "Doubling on C",      "HG C D" },
      { "Doubling on D",      "HG D E" },
      { "Doubling on E",      "HG E F" },
      { "Doubling on F",      "HG F HG" },

      // Thumb Doublings
      { "Thumb doubling on low G", "HA LG D" },
      { "Thumb doubling on low A", "HA LA D" },
      { "Thumb doubling on B", "HA B D" },
      { "Thumb doubling on C", "HA C D" },
      { "Thumb doubling on D", "HA D E" },
      { "Thumb doubling on E", "HA E F" },
      { "Thumb doubling on F", "HA F HG" },
      // ? { "Thumb doubling on high G", "HA HG F" },

      // G Grace note Strikes
      { "G grace note on low A", "HG LA LG" },
      { "G grace note on B", "HG B LG" },
      { "G grace note on C", "HG C LG" },
      { "G grace note on D", "HG D LG" },
      { "G grace note on D", "HG D C" },
      { "G grace note on E", "HG E LA" },
      { "G grace note on F", "HG F E" },
      
      // Regular Double Strikes
      { "Double strike on low A", "LG LA LG" },
      { "Double strike on B", "LG B LG" },
      { "Double strike on C", "LG C LG" },
      { "Double strike on D", "LG D LG" },
      { "Double strike on D", "C D C" },
      { "Double strike on E", "LA E LA" },
      { "Double strike on F", "E F E" },
      { "Double strike on high G", "F HG F" },
      { "Double strike on high A", "HG HA HG" },

      // Thumb Strikes
      { "Thumb strike on low A", "HA LA LG" },
      { "Thumb strike on B", "HA B LG" },
      { "Thumb strike on C", "HA C LG" },
      { "Thumb strike on D", "HA D LG" },
      { "Thumb strike on D", "HA D C" },
      { "Thumb strike on E", "HA E LA" },
      { "Thumb strike on F", "HA F E" },
      { "Thumb strike on high G", "HA HG F" },

      // Regular Grips (continued)
      { "Grip", "LG D LG" },
      { "Grip", "LG B LG" },

      // Taorluath and Birl
      { "Birl", "LG LA LG" },
      { "D throw", "LG D C" },
      { "Half heavy D throw", "D LG C" },
      { "Taorluath", "D LG E" },

      // Birl, Bubly, D Throws (continued) and Taorluaths (continued)
      { "Birl", "LA LG LA LG" },
      { "Bubly", "D LG C LG" },
      { "Heavy D throw", "LG D LG C" },
      { "Taorluath", "LG D LG E" },
      { "Taorluath", "LG B LG E" },
      
      // Half Double Strikes
      { "Half double strike on low A", "LA LG LA LG" },
      { "Half double strike on B", "B LG B LG" },
      { "Half double strike on C", "C LG C LG" },
      { "Half double strike on D", "D LG D LG" },
      { "Half double strike on D", "D C D C" },
      { "Half double strike on E", "E LA E LA" },
      { "Half double strike on F", "F E F E" },
      { "Half double strike on high G", "HG F HG F" },
      { "Half double strike on high A", "HA HG HA HG" },
      
      // Half Grips
      { "Half grip on low A", "LA LG D LG" },
      { "Half grip on B", "B LG D LG" },
      { "Half grip on C", "C LG D LG" },
      { "Half grip on D", "D LG D LG" },
      { "Half grip on D", "D LG B LG" },
      { "Half grip on E", "E LG D LG" },
      { "Half grip on F", "F LG F LG" },
      { "Half grip on high G", "HG LG D LG" },
      { "Half grip on high A", "HA LG D LG" },
      
      // Half Peles
      { "Half pele on low A", "LA E LA LG" },
      { "Half pele on B", "B E B LG" },
      { "Half pele on C", "C E C LG" },
      { "Half pele on D", "D E D LG" },
      { "Half pele on D", "D E D C" },
      { "Half pele on E", "E F E LA" },
      { "Half pele on F", "F HG F E" },
      { "Half pele on high G", "HG HA HG F" },

      // G Grace note Grips
      { "G grace note grip on low A", "HG LA LG D LG" },
      { "G grace note grip on B", "HG B LG D LG" },
      { "G grace note grip on C", "HG C LG D LG" },
      { "G grace note grip on D", "HG D LG D LG" },
      { "G grace note grip on D", "HG D LG B LG" },
      { "G grace note grip on E", "HG E LG D LG" },
      { "G grace note grip on F", "HG F LG F LG" },

      // Thumb Grips
      { "Thumb grip on low A", "HA LA LG D LG" },
      { "Thumb grip on B", "HA B LG D LG" },
      { "Thumb grip on C", "HA C LG D LG" },
      { "Thumb grip on D", "HA D LG D LG" },
      { "Thumb grip on D", "HA D LG B LG" },
      { "Thumb grip on E", "HA E LG D LG" },
      { "Thumb grip on F", "HA F LG F LG" },
      { "Thumb grip on high G", "HA HG LG F LG" },

      // Bubly
      { "Bubly", "LG D LG C LG" },

      //  Birls
      { "Birl", "HG LA LG LA LG" },
      { "Birl", "HA LA LG LA LG" },

      // Regular Peles
      { "Pele on low A", "HG LA E LA LG" },
      { "Pele on B", "HG B E B LG" },
      { "Pele on C", "HG C E C LG" },
      { "Pele on D", "HG D E D LG" },
      { "Pele on D", "HG D E D C" },
      { "Pele on E", "HG E F E LA" },
      { "Pele on F", "HG F HG F E" },
 
      // Thumb Grace Note Peles
      { "Thumb grace note pele on low A", "HA LA E LA LG" },
      { "Thumb grace note pele on B", "HA B E B LG" },
      { "Thumb grace note pele on C", "HA C E C LG" },
      { "Thumb grace note pele on D", "HA D E D LG" },
      { "Thumb grace note pele on D", "HA D E D C" },
      { "Thumb grace note pele on E", "HA E F E LA" },
      { "Thumb grace note pele on F", "HA F HG F E" },
      { "Thumb grace note pele on high G", "HA HG HA HG F" },

      // G Grace note Double Strikes
      { "G grace note double strike on low A", "HG LA LG LA LG" },
      { "G grace note double strike on B", "HG B LG B LG" },
      { "G grace note double strike on C", "HG C LG C LG" },
      { "G grace note double strike on D", "HG D LG D LG" },
      { "G grace note double strike on D", "HG D C D C" },
      { "G grace note double strike on E", "HG E LA E LA" },
      { "G grace note double strike on F", "HG F E F E" },

      // Thumb Double Strikes
      { "Thumb double strike on low A", "HA LA LG LA LG" },
      { "Thumb double strike on B", "HA B LG B LG" },
      { "Thumb double strike on C", "HA C LG C LG" },
      { "Thumb double strike on D", "HA D LG D LG" },
      { "Thumb double strike on D", "HA D C D C" },
      { "Thumb double strike on E", "HA E LA E LA" },
      { "Thumb double strike on F", "HA F E F E" },
      { "Thumb double strike on high G", "HA HG F HG F" },

      // Regular Triple Strikes
      { "Triple strike on low A", "LG LA LG LA LG" },
      { "Triple strike on B", "LG B LG B LG" },
      { "Triple strike on C", "LG C LG C LG" },
      { "Triple strike on D", "LG D LG D LG" },
      { "Triple strike on D", "C D C D C" },
      { "Triple strike on E", "LA E LA E LA" },
      { "Triple strike on F", "E F E F E" },
      { "Triple strike on high G", "F HG F HG F" },
      { "Triple strike on high A", "HG HA HG HA HG" },
      
      // Half Triple Strikes
      { "Half triple strike on low A", "LA LG LA LG LA LG" },
      { "Half triple strike on B", "B LG B LG B LG" },
      { "Half triple strike on C", "C LG C LG C LG" },
      { "Half triple strike on D", "D LG D LG D LG" },
      { "Half triple strike on D", "D C D C D C" },
      { "Half triple strike on E", "E LA E LA E LA" },
      { "Half triple strike on F", "F E F E F E" },
      { "Half triple strike on high G", "HG F HG F HG F" },
      { "Half triple strike on high A", "HA HG HA HG HA HG" },

      // G Grace note Triple Strikes
      { "G grace note triple strike on low A", "HG LA LG LA LG LA LG" },
      { "G grace note triple strike on B", "HG B LG B LG B LG" },
      { "G grace note triple strike on C", "HG C LG C LG C LG" },
      { "G grace note triple strike on D", "HG D LG D LG D LG" },
      { "G grace note triple strike on D", "HG D C D C D C" },
      { "G grace note triple strike on E", "HG E LA E LA E LA" },
      { "G grace note triple strike on F", "HG F E F E F E" },

      // Thumb Triple Strikes
      { "Thumb triple strike on low A",  "HA LA LG LA LG LA LG" },
      { "Thumb triple strike on B",      "HA B LG B LG B LG" },
      { "Thumb triple strike on C",      "HA C LG C LG C LG" },
      { "Thumb triple strike on D",      "HA D LG D LG D LG" },
      { "Thumb triple strike on D",      "HA D C D C D C" },
      { "Thumb triple strike on E",      "HA E LA E LA E LA" },
      { "Thumb triple strike on F",      "HA F E F E F E" },
      { "Thumb triple strike on high G", "HA HG F HG F HG F" },

      };

// Staff line and pitch for every bagpipe note

BagpipeNoteInfo BagpipeEmbellishment::BagpipeNoteInfoList[] = {
      { "LG",  6,  67},
      { "LA",  5,  69},
      { "B",   4,  71},
      { "C",   3,  73}, // actually C#
      { "D",   2,  74},
      { "E",   1,  76},
      { "F",   0,  78}, // actually F#
      { "HG", -1,  79},
      { "HA", -2,  81}
};

//---------------------------------------------------------
//   nEmbellishments
//     return the number of embellishment in BagpipeEmbellishmentList
//---------------------------------------------------------

int BagpipeEmbellishment::nEmbellishments()
      {
      return sizeof(BagpipeEmbellishmentList) / sizeof(*BagpipeEmbellishmentList);
      }
      
//---------------------------------------------------------
//   getNoteList
//     return notes as list of indices in BagpipeNoteInfoList
//---------------------------------------------------------

noteList BagpipeEmbellishment::getNoteList() const
      {
      noteList nl;
      if (_embelType >= 0 && _embelType < nEmbellishments()) {
            QStringList notes = BagpipeEmbellishmentList[_embelType].notes.split(' ');
            int noteInfoSize = sizeof(BagpipeNoteInfoList) / sizeof(*BagpipeNoteInfoList);
            foreach (const QString note, notes) {
                  // search for note in BagpipeNoteInfoList
                  for (int i = 0; i < noteInfoSize; ++i) {
                        if (BagpipeNoteInfoList[i].name == note) {
                              // found it, append to list
                              nl << i;
                              break;
                              }
                        }
                  }
            }
      return nl;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BagpipeEmbellishment::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", _embelType);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BagpipeEmbellishment::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _embelType = e.readInt();
            else
                  e.unknown();
            }
      }
      
//---------------------------------------------------------
//   BEDrawingDataX
//      BagpipeEmbellishment drawing data in the x direction
//      shared between ::draw() and ::layout()
//---------------------------------------------------------
      
struct BEDrawingDataX {
      const Sym& headsym;     // grace note head symbol
      const Sym& flagsym;     // grace note flag symbol
      const qreal mags;       // grace head magnification
      const qreal headw;      // grace head width
      const qreal headp;      // horizontal head pitch
      const qreal spatium;    // spatium
      const qreal lw;         // line width for stem
      const qreal xl;         // calc x for stem of leftmost note
      const qreal xcorr;      // correction to align flag with top of stem

      BEDrawingDataX(const Sym& hs, const Sym& fs, const qreal m, const qreal s, const int nn)
         :  headsym(hs),
            flagsym(fs),
            mags(     0.75 * m),
            headw(    hs.width(mags)),
            headp(    1.4 * hs.width(mags)),
            spatium(  s),
            lw(       0.1 * s),
            xl(       (1 - 1.4 * (nn - 1)) * hs.width(mags) / 2),
            xcorr(    0.1 * s) {}
};
      
//---------------------------------------------------------
//   BEDrawingDataY
//      BagpipeEmbellishment drawing data in the y direction
//      shared between ::draw() and ::layout()
//---------------------------------------------------------

struct BEDrawingDataY {
      const qreal y1b;        // top of all stems for beamed notes
      const qreal y1f;        // top of stem for note with flag
      const qreal y2;         // bottom of stem
      const qreal ycorr;      // correction to align flag with top of stem
      const qreal bw;         // line width for beam
      
      BEDrawingDataY(const int l, const qreal s)
         :  y1b(   -8 * s / 2),
            y1f(    (l - 6) * s / 2),
            y2(     l * s / 2),
            ycorr( -0.2 * s),
            bw(     0.3 * s) {}
};
      
//---------------------------------------------------------
//   debug support (disabled)
//---------------------------------------------------------

static void printBBox(const char* name, const QRectF b)
      {
      /*
      qDebug("bbox%s left %f bot %f right %f top %f",
             name,
             b.left(),
             b.bottom(),
             b.right(),
             b.top());
       */
      }

static void symMetrics(const char* name, const Sym& headsym)
      {
      /*
      qDebug("%s", name);
      qDebug("bbox left %f bot %f right %f top %f",
             headsym.getBbox().left(),
             headsym.getBbox().bottom(),
             headsym.getBbox().right(),
             headsym.getBbox().top());
      qDebug("attach x %f y %f",
             headsym.getAttach().x(),
             headsym.getAttach().y());
       */
      }
      
//---------------------------------------------------------
//   layout
//      calculate and set bounding box
//---------------------------------------------------------

void BagpipeEmbellishment::layout()
      {
      if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
            // qDebug("BagpipeEmbellishment::layout st %d", _embelType);
      }
      const Sym& headsym = symbols[score()->symIdx()][quartheadSym];
      const Sym& flagsym = symbols[score()->symIdx()][thirtysecondflagSym];
            
      noteList nl = getNoteList();
      BEDrawingDataX dx(headsym, flagsym, magS(), score()->spatium(), nl.size());

      setbbox(QRectF());
      if (_embelType == 0) {
            symMetrics("headsym", headsym);
            symMetrics("flagsym", flagsym);
            // qDebug("mags %f headw %f headp %f spatium %f", dx.mags, dx.headw, dx.headp, dx.spatium);
            }
            
      bool drawFlag = nl.size() == 1;
            
      // draw the notes including stem, (optional) flag and (optional) ledger line
      qreal x = dx.xl;
      foreach (int note, nl) {
            int line = BagpipeNoteInfoList[note].line;
            BEDrawingDataY dy(line, score()->spatium());

            // head
            addbbox(headsym.bbox(dx.mags).translated(QPointF(x - dx.lw * .5 - dx.headw, dy.y2)));
            if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
                  printBBox(" notehead", bbox());
                  }

            // stem
            // highest top of stems actually used is y1b
            addbbox(QRectF(x - dx.lw * .5 - dx.headw, dy.y1b, dx.lw, dy.y2 - dy.y1b));
            if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
                  printBBox(" notehead + stem", bbox());
                  }

            // flag
            if (drawFlag) {
                  addbbox(flagsym.bbox(dx.mags).translated(QPointF(x - dx.lw * .5 + dx.xcorr, dy.y1f + dy.ycorr)));
                  printBBox(" notehead + stem + flag", bbox());
                  }

            // draw the ledger line for high A
            if (line == -2) {
                  addbbox(QRectF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2 - dx.lw * 2, dx.headw * 2, dx.lw));
                  if (_embelType == 8) {
                        printBBox(" notehead + stem + ledger line", bbox());
                        }
                  }
            
            // move x to next note x position
            x += dx.headp;
            }
      }

//---------------------------------------------------------
//   drawBeams
//      draw the beams
//      x1,y is one side of the top beam
//      x2,y is the other side of the top beam
//---------------------------------------------------------
      
static void drawBeams(QPainter* painter, const qreal spatium,
                      const qreal x1, const qreal x2, qreal y)
      {
      // draw the beams
      painter->drawLine(QLineF(x1, y, x2, y));
      y += spatium / 1.5;
      painter->drawLine(QLineF(x1, y, x2, y));
      y += spatium / 1.5;
      painter->drawLine(QLineF(x1, y, x2, y));
      }

//---------------------------------------------------------
//   drawGraceNote
//      draw a single grace note in a palette cell
//      x,y1 is the top of the stem
//      x,y2 is the bottom of the stem
//---------------------------------------------------------

static void drawGraceNote(QPainter* painter, const BEDrawingDataX& dx, const BEDrawingDataY& dy,
                          const Sym& flagsym, const qreal x, const bool drawFlag)
      {
      // draw head
      dx.headsym.draw(painter, dx.mags, QPointF(x - dx.headw, dy.y2));
      // draw stem
      qreal y1 =  drawFlag ? dy.y1f : dy.y1b;          // top of stems actually used
      painter->drawLine(QLineF(x - dx.lw * .5, y1, x - dx.lw * .5, dy.y2));
      if (drawFlag) {
            // draw flag
            flagsym.draw(painter, dx.mags, QPointF(x - dx.lw * .5 + dx.xcorr, y1 + dy.ycorr));
            }
      }

//---------------------------------------------------------
//   draw
//      draw the embellishment centered in a palette cell
//      x = 0 is horizontal cell center
//      y = 0 is the top staff line
//---------------------------------------------------------

void BagpipeEmbellishment::draw(QPainter* painter) const
      {
      const Sym& headsym = symbols[score()->symIdx()][quartheadSym];
      const Sym& flagsym = symbols[score()->symIdx()][thirtysecondflagSym];

      noteList nl = getNoteList();
      BEDrawingDataX dx(headsym, flagsym, magS(), score()->spatium(), nl.size());

      QPen pen(curColor(), dx.lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);

      bool drawBeam = nl.size() > 1;
      bool drawFlag = nl.size() == 1;

      // draw the notes including stem, (optional) flag and (optional) ledger line
      qreal x = dx.xl;
      foreach (int note, nl) {
            int line = BagpipeNoteInfoList[note].line;
            BEDrawingDataY dy(line, score()->spatium());
            drawGraceNote(painter, dx, dy, flagsym, x, drawFlag);

            // draw the ledger line for high A
            if (line == -2)
                  painter->drawLine(QLineF(x - dx.headw * 1.5 - dx.lw * .5,
                                           dy.y2, x + dx.headw * .5 - dx.lw * .5, dy.y2));

            // move x to next note x position
            x += dx.headp;
            }

      if (drawBeam) {
            // beam drawing setup
            BEDrawingDataY dy(0, score()->spatium());
            QPen beamPen(curColor(), dy.bw, Qt::SolidLine, Qt::FlatCap);
            painter->setPen(beamPen);
            // draw the beams
            drawBeams(painter, dx.spatium, dx.xl - dx.lw * .5, x - dx.headp - dx.lw * .5, dy.y1b);
            }
      }
}
