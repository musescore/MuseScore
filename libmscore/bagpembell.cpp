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
//   drawGraceNote
//      draw a single grace note in a palette cell
//      x,y1 is the top of the stem
//      x,y2 is the bottom of the stem
//---------------------------------------------------------

static void drawGraceNote(QPainter* painter, const Sym& headsym, const qreal lw, const qreal mags,
                          const qreal x, const qreal y1, const qreal y2)
      {
      qreal headw = headsym.width(mags);
      painter->drawLine(QLineF(x - lw * .5, y1, x - lw * .5, y2));
      headsym.draw(painter, mags, QPointF(x - headw, y2));
      }

//---------------------------------------------------------
//   draw
//      draw the embellishment in a palette cell
//      x = 0 is horizontal cell center
//      y = 0 is the top staff line
//---------------------------------------------------------

void BagpipeEmbellishment::draw(QPainter* painter) const
      {
      const Sym& headsym = symbols[score()->symIdx()][quartheadSym];
      qreal mags = magS() * 0.75;         // grace head magnification
      qreal headw = headsym.width(mags);  // grace head width
      qreal headp = headw * 1.4;          // horizontal head pitch
      qreal _spatium = score()->spatium();
      
      qreal lw = 0.1 * _spatium;          // line width for stem
      qreal y1 =  -8 * _spatium / 2;      // top of stems
      
      QPen pen(curColor(), lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);

      noteList nl = getNoteList();
      int note;
      // calc x for stem of leftmost note
      // note embellishment is centered in the palette cell
      qreal x  = (headw - headp * (nl.size() - 1)) / 2;
      qreal x0 = x; // remember for later (left side of beam)
      // draw the notes including stems
      foreach (note, nl) {
            int line = BagpipeNoteInfoList[note].line;
            qreal y2 = line * _spatium / 2;
            drawGraceNote(painter, headsym, lw, mags, x, y1, y2);
            if (line == -2)
                  // draw the ledger line for high A
                  painter->drawLine(QLineF(x - headw * 1.5 - lw * .5, y2, x + headw * .5 - lw * .5, y2));
            x += headp;
            }

      // beam drawing setup
      qreal bw = 0.3 * _spatium;          // line width for beam
      QPen beamPen(curColor(), bw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(beamPen);
      // simplification: use short beam instead of flag for single grace notes
      // TODO: replace by flags and fix stem length for single grace notes
      if (nl.size() == 1)
            x += 0.75 * headp;
      // draw the beam
      painter->drawLine(QLineF(x0 - lw * .5, y1, x - headp - lw * .5, y1));
      y1 += _spatium / 1.5;
      painter->drawLine(QLineF(x0 - lw * .5, y1, x - headp - lw * .5, y1));
      y1 += _spatium / 1.5;
      painter->drawLine(QLineF(x0 - lw * .5, y1, x - headp - lw * .5, y1));
      }

}
