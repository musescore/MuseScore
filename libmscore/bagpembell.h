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

#ifndef __BAGPEMBELL_H__
#define __BAGPEMBELL_H__

#include "element.h"

namespace Ms {

typedef QList<int> noteList;

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name and notes for a BagpipeEmbellishment
//---------------------------------------------------------

struct BagpipeEmbellishmentInfo {
      const char* name;
      QString notes;
      };

//---------------------------------------------------------
//   BagpipeEmbellishmentInfo
//    name, staff line and pitch for a bagpipe note
//---------------------------------------------------------

struct BagpipeNoteInfo {
      QString name;
      int line;
      int pitch;
      };

struct BEDrawingDataX;
struct BEDrawingDataY;

//---------------------------------------------------------
//   BagpipeEmbellishment
//    dummy element, used for drag&drop
//---------------------------------------------------------

class BagpipeEmbellishment final : public Element {
      int _embelType;
      void drawGraceNote(QPainter*, const BEDrawingDataX&, const BEDrawingDataY&,
         SymId, const qreal x, const bool drawFlag) const;

   public:
      BagpipeEmbellishment(Score* s) : Element(s), _embelType(0) { }
      virtual BagpipeEmbellishment* clone() const override { return new BagpipeEmbellishment(*this); }
      virtual ElementType type() const override            { return ElementType::BAGPIPE_EMBELLISHMENT;           }
      int embelType() const                                { return _embelType;                      }
      void setEmbelType(int val)                           { _embelType = val;                       }
      virtual qreal mag() const override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      static BagpipeEmbellishmentInfo BagpipeEmbellishmentList[];
      static int nEmbellishments();
      static BagpipeNoteInfo BagpipeNoteInfoList[];
      noteList getNoteList() const;
      };


}     // namespace Ms
#endif

