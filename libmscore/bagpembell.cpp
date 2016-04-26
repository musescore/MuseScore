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
#include "sym.h"

namespace Ms {

// Embellishment names and note sequences

BagpipeEmbellishmentInfo BagpipeEmbellishment::BagpipeEmbellishmentList[] = {

      // Single Grace notes
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace low G"), "LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace low A"), "LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace B"), "B" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace C"), "C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace D"), "D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace E"), "E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace F"), "F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace high G"), "HG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Single grace high A"), "HA" },

      // Double Grace notes
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "D LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "D B" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "E LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "E B" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "E C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "E D" },

      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "F LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "F LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "F B" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "F C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "F D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "F E" },

      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG B" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HG F" },

      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA B" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double grace"), "HA HG" },

      // Half Doublings
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on low G"), "LG D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on low A"), "LA D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on B"), "B D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on C"), "C D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on D"), "D E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on E"), "E F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on F"), "F HG" },
      // ? { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on high G"), "HG F" },
      // ? { QT_TRANSLATE_NOOP("bagpipe", "Half doubling on high A"), "HA HG" },

      // Regular Doublings
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on high G"), "HG F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on high A"), "HA HG" },

      // Half Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on low A"), "LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on B"), "B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on C"), "C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on D"), "D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on D"), "D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on E"), "E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on F"), "F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half strike on high G"), "HG F" },

      // Regular Grip
      { QT_TRANSLATE_NOOP("bagpipe", "Grip"), "D LG" },

      // D Throw
      { QT_TRANSLATE_NOOP("bagpipe", "Half D throw"), "D C" },

      // Regular Doublings (continued)
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on low G"),  "HG LG D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on low A"),  "HG LA D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on B"),      "HG B D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on C"),      "HG C D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on D"),      "HG D E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on E"),      "HG E F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Doubling on F"),      "HG F HG" },

      // Thumb Doublings
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on low G"), "HA LG D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on low A"), "HA LA D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on B"), "HA B D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on C"), "HA C D" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on D"), "HA D E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on E"), "HA E F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on F"), "HA F HG" },
      // ? { QT_TRANSLATE_NOOP("bagpipe", "Thumb doubling on high G"), "HA HG F" },

      // G Grace note Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on low A"), "HG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on B"), "HG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on C"), "HG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on D"), "HG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on D"), "HG D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on E"), "HG E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note on F"), "HG F E" },

      // Regular Double Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on low A"), "LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on B"), "LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on C"), "LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on D"), "LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on D"), "C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on E"), "LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on F"), "E F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on high G"), "F HG F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Double strike on high A"), "HG HA HG" },

      // Thumb Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on low A"), "HA LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on B"), "HA B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on C"), "HA C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on D"), "HA D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on D"), "HA D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on E"), "HA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on F"), "HA F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb strike on high G"), "HA HG F" },

      // Regular Grips (continued)
      { QT_TRANSLATE_NOOP("bagpipe", "Grip"), "LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Grip"), "LG B LG" },

      // Taorluath and Birl
      { QT_TRANSLATE_NOOP("bagpipe", "Birl"), "LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "D throw"), "LG D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half heavy D throw"), "D LG C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Taorluath"), "D LG E" },

      // Birl, Bubly, D Throws (continued) and Taorluaths (continued)
      { QT_TRANSLATE_NOOP("bagpipe", "Birl"), "LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Bubly"), "D LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Heavy D throw"), "LG D LG C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Taorluath"), "LG D LG E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Taorluath"), "LG B LG E" },

      // Half Double Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on low A"), "LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on B"), "B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on C"), "C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on D"), "D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on D"), "D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on E"), "E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on F"), "F E F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on high G"), "HG F HG F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half double strike on high A"), "HA HG HA HG" },

      // Half Grips
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on low A"), "LA LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on B"), "B LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on C"), "C LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on D"), "D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on D"), "D LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on E"), "E LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on F"), "F LG F LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on high G"), "HG LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half grip on high A"), "HA LG D LG" },

      // Half Peles
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on low A"), "LA E LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on B"), "B E B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on C"), "C E C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on D"), "D E D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on D"), "D E D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on E"), "E F E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on F"), "F HG F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half pele on high G"), "HG HA HG F" },

      // G Grace note Grips
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on low A"), "HG LA LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on B"), "HG B LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on C"), "HG C LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on D"), "HG D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on D"), "HG D LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on E"), "HG E LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note grip on F"), "HG F LG F LG" },

      // Thumb Grips
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on low A"), "HA LA LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on B"), "HA B LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on C"), "HA C LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on D"), "HA D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on D"), "HA D LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on E"), "HA E LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on F"), "HA F LG F LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grip on high G"), "HA HG LG F LG" },

      // Bubly
      { QT_TRANSLATE_NOOP("bagpipe", "Bubly"), "LG D LG C LG" },

      //  Birls
      { QT_TRANSLATE_NOOP("bagpipe", "Birl"), "HG LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Birl"), "HA LA LG LA LG" },

      // Regular Peles
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on low A"), "HG LA E LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on B"), "HG B E B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on C"), "HG C E C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on D"), "HG D E D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on D"), "HG D E D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on E"), "HG E F E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Pele on F"), "HG F HG F E" },

      // Thumb Grace Note Peles
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on low A"), "HA LA E LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on B"), "HA B E B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on C"), "HA C E C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on D"), "HA D E D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on D"), "HA D E D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on E"), "HA E F E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on F"), "HA F HG F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb grace note pele on high G"), "HA HG HA HG F" },

      // G Grace note Double Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on low A"), "HG LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on B"), "HG B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on C"), "HG C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on D"), "HG D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on D"), "HG D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on E"), "HG E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note double strike on F"), "HG F E F E" },

      // Thumb Double Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on low A"), "HA LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on B"), "HA B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on C"), "HA C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on D"), "HA D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on D"), "HA D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on E"), "HA E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on F"), "HA F E F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb double strike on high G"), "HA HG F HG F" },

      // Regular Triple Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on low A"), "LG LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on B"), "LG B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on C"), "LG C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on D"), "LG D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on D"), "C D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on E"), "LA E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on F"), "E F E F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on high G"), "F HG F HG F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Triple strike on high A"), "HG HA HG HA HG" },

      // Half Triple Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on low A"), "LA LG LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on B"), "B LG B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on C"), "C LG C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on D"), "D LG D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on D"), "D C D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on E"), "E LA E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on F"), "F E F E F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on high G"), "HG F HG F HG F" },
      { QT_TRANSLATE_NOOP("bagpipe", "Half triple strike on high A"), "HA HG HA HG HA HG" },

      // G Grace note Triple Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on low A"), "HG LA LG LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on B"), "HG B LG B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on C"), "HG C LG C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on D"), "HG D LG D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on D"), "HG D C D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on E"), "HG E LA E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "G grace note triple strike on F"), "HG F E F E F E" },

      // Thumb Triple Strikes
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on low A"),  "HA LA LG LA LG LA LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on B"),      "HA B LG B LG B LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on C"),      "HA C LG C LG C LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on D"),      "HA D LG D LG D LG" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on D"),      "HA D C D C D C" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on E"),      "HA E LA E LA E LA" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on F"),      "HA F E F E F E" },
      { QT_TRANSLATE_NOOP("bagpipe", "Thumb triple strike on high G"), "HA HG F HG F HG F" },

      };

// Staff line and pitch for every bagpipe note
BagpipeNoteInfo BagpipeEmbellishment::BagpipeNoteInfoList[] = {
      { "LG",  6,  65},
      { "LA",  5,  67},
      { "B",   4,  69},
      { "C",   3,  71}, // actually C#
      { "D",   2,  72},
      { "E",   1,  74},
      { "F",   0,  76}, // actually F#
      { "HG", -1,  77},
      { "HA", -2,  79}
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
      const SymId headsym;    // grace note head symbol
      const SymId flagsym;    // grace note flag symbol
      const qreal mags;       // grace head magnification
      qreal headw;            // grace head width
      qreal headp;            // horizontal head pitch
      const qreal spatium;    // spatium
      const qreal lw;         // line width for stem
      qreal xl;               // calc x for stem of leftmost note
      const qreal xcorr;      // correction to align flag with top of stem

      BEDrawingDataX(SymId hs, SymId fs, const qreal m, const qreal s, const int nn)
         :  headsym(hs),
            flagsym(fs),
            mags(   0.75 * m),
            spatium(s),
            lw(     0.1 * s),
            xcorr(  0.1 * s)
            {
            qreal w = gscore->scoreFont()->width(hs, mags);
            headw = 1.2 * w; // using 1.0 the stem xpos is off
            headp = 1.6 * w;
            xl    = (1 - 1.6 * (nn - 1)) * w / 2;
            }
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
         :  y1b( -8 * s / 2),
            y1f( (l - 6) * s / 2),
            y2(   l * s / 2),
            ycorr(0.8 * s),
            bw(   0.3 * s) {}
};

//---------------------------------------------------------
//   debug support (disabled)
//---------------------------------------------------------

/*
static void printBBox(const char* name, const QRectF b)
      {
      qDebug("bbox%s left %f bot %f right %f top %f",
             name,
             b.left(),
             b.bottom(),
             b.right(),
             b.top());
      }

static void symMetrics(const char* name, const Sym& headsym)
      {
      qDebug("%s", name);
      qDebug("bbox left %f bot %f right %f top %f",
             headsym.getBbox().left(),
             headsym.getBbox().bottom(),
             headsym.getBbox().right(),
             headsym.getBbox().top());
      qDebug("attach x %f y %f",
             headsym.getAttach().x(),
             headsym.getAttach().y());
      }
*/
      
//---------------------------------------------------------
//   mag
//      return fixed magnification
//---------------------------------------------------------

qreal BagpipeEmbellishment::mag() const
      {
            return 0.7;
      }

//---------------------------------------------------------
//   layout
//      calculate and set bounding box
//---------------------------------------------------------

void BagpipeEmbellishment::layout()
      {
      /*
      if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
            qDebug("BagpipeEmbellishment::layout st %d", _embelType);
            }
       */
      SymId headsym = SymId::noteheadBlack;
      SymId flagsym = SymId::flag32ndUp;

      noteList nl = getNoteList();
      BEDrawingDataX dx(headsym, flagsym, magS(), score()->spatium(), nl.size());

      setbbox(QRectF());
      /*
      if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
            symMetrics("headsym", headsym);
            symMetrics("flagsym", flagsym);
            qDebug("mags %f headw %f headp %f spatium %f xl %f",
                   dx.mags, dx.headw, dx.headp, dx.spatium, dx.xl);
            }
       */

      bool drawFlag = nl.size() == 1;

      // draw the notes including stem, (optional) flag and (optional) ledger line
      qreal x = dx.xl;
      foreach (int note, nl) {
            int line = BagpipeNoteInfoList[note].line;
            BEDrawingDataY dy(line, score()->spatium());

            // head
            addbbox(score()->scoreFont()->bbox(headsym, dx.mags).translated(QPointF(x - dx.lw * .5 - dx.headw, dy.y2)));
            /*
            if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
                  printBBox(" notehead", bbox());
                  }
             */

            // stem
            // highest top of stems actually used is y1b
            addbbox(QRectF(x - dx.lw * .5 - dx.headw, dy.y1b, dx.lw, dy.y2 - dy.y1b));
            /*
            if (_embelType == 0 || _embelType == 8 || _embelType == 9) {
                  printBBox(" notehead + stem", bbox());
                  }
             */

            // flag
            if (drawFlag) {
                  addbbox(score()->scoreFont()->bbox(flagsym, dx.mags).translated(QPointF(x - dx.lw * .5 + dx.xcorr, dy.y1f + dy.ycorr)));
                  // printBBox(" notehead + stem + flag", bbox());
                  }

            // draw the ledger line for high A
            if (line == -2) {
                  addbbox(QRectF(x - dx.headw * 1.5 - dx.lw * .5, dy.y2 - dx.lw * 2, dx.headw * 2, dx.lw));
                  /*
                  if (_embelType == 8) {
                        printBBox(" notehead + stem + ledger line", bbox());
                        }
                   */
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

void BagpipeEmbellishment::drawGraceNote(QPainter* painter,
   const BEDrawingDataX& dx,
   const BEDrawingDataY& dy,
   SymId flagsym, const qreal x, const bool drawFlag) const
      {
      // draw head
      drawSymbol(dx.headsym, painter, QPointF(x - dx.headw, dy.y2));
      // draw stem
      qreal y1 =  drawFlag ? dy.y1f : dy.y1b;          // top of stems actually used
      painter->drawLine(QLineF(x - dx.lw * .5, y1, x - dx.lw * .5, dy.y2));
      if (drawFlag) {
            // draw flag
            drawSymbol(flagsym, painter, QPointF(x - dx.lw * .5 + dx.xcorr, y1 + dy.ycorr));
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
      SymId headsym = SymId::noteheadBlack;
      SymId flagsym = SymId::flag32ndUp;

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
