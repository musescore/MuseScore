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

      // Regular Doublings
      { "Doubling on low G",  "HG LG D" },
      { "Doubling on low A",  "HG LA D" },
      { "Doubling on B",      "HG B D" },
      { "Doubling on C",      "HG C D" },
      { "Doubling on D",      "HG D E" },
      { "Doubling on E",      "HG E F" },
      { "Doubling on F",      "HG F HG" },
      { "Doubling on high G", "HG F" },
      { "Doubling on high A", "HA HG" },

      // G Grace note, Thumb and Half Triple Strikes
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
      
      qreal lw = 0.1 * _spatium;
      qreal y1 =  -6 * _spatium / 2;
      
      QPen pen(curColor(), lw, Qt::SolidLine, Qt::FlatCap);
      painter->setPen(pen);

      noteList nl = getNoteList();
      int note;
      // calc x for stem of leftmost note
      // embellishment is centered in the cell
      qreal x  = (headw - headp * (nl.size() - 1)) / 2;
      qreal x0 = x; // remember for later
      // draw the notes including stems
      foreach (note, nl) {
            qreal y2 = BagpipeNoteInfoList[note].line * _spatium / 2;
            drawGraceNote(painter, headsym, lw, mags, x, y1, y2);
            x += headp;
            }
      // draw the beam
      painter->drawLine(QLineF(x0 - lw * .5, y1, x - headp - lw * .5, y1));
      }

}
