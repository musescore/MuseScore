//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __AMBITUS_H__
#define __AMBITUS_H__

#include "element.h"
#include "note.h"
#include "accidental.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Ambitus
//---------------------------------------------------------

class Ambitus final : public Element {
      NoteHead::Group     _noteHeadGroup;
      NoteHead::Type      _noteHeadType;
      MScore::DirectionH  _dir;
      bool  _hasLine;
      Spatium _lineWidth;
      Accidental  _topAccid, _bottomAccid;
      int   _topPitch, _bottomPitch;
      int   _topTpc, _bottomTpc;

      // internally managed, to optimize layout / drawing
      QPointF _topPos;                          // position of top note symbol
      QPointF _bottomPos;                       // position of bottom note symbol
      QLineF  _line;                            // the drawn line

      void  normalize();

   public:
      Ambitus(Score* s);
      virtual Ambitus* clone() const override         { return new Ambitus(*this); }

      void initFrom(Ambitus* a);

      // getters and setters
      virtual ElementType type() const override       { return ElementType::AMBITUS; }
      NoteHead::Group noteHeadGroup() const           { return _noteHeadGroup;}
      NoteHead::Type noteHeadType() const             { return _noteHeadType; }
      MScore::DirectionH direction() const            { return _dir;          }
      bool hasLine() const                            { return _hasLine;      }
      Spatium lineWidth() const                       { return _lineWidth;    }
      int topOctave() const                           { return (_topPitch / 12) - 1; }
      int bottomOctave() const                        { return (_bottomPitch / 12) - 1; }
      int topPitch() const                            { return _topPitch;     }
      int bottomPitch() const                         { return _bottomPitch;  }
      int topTpc() const                              { return _topTpc;       }
      int bottomTpc() const                           { return _bottomTpc;    }

      void setNoteHeadGroup(NoteHead::Group val)      { _noteHeadGroup = val; }
      void setNoteHeadType (NoteHead::Type val)       { _noteHeadType  = val; }
      void setDirection    (MScore::DirectionH val)   { _dir = val;           }
      void setHasLine      (bool val)                 { _hasLine = val;       }
      void setLineWidth    (Spatium val)              { _lineWidth = val;     }
      void setTopPitch     (int val);
      void setBottomPitch  (int val);
      void setTopTpc       (int val);
      void setBottomTpc    (int val);

      // some utility functions
      Segment* segment() const                        { return (Segment*)parent(); }
      SymId noteHead() const;
      qreal headWidth() const;
      void  updateRange();                // scan staff up to next section break and update range pitches

      // re-implemented virtual functions
      virtual void      draw(QPainter*) const override;
      virtual void      layout() override;
      virtual QPointF   pagePos() const override;      ///< position in page coordinates
      virtual void      read(XmlReader&) override;
      virtual void      scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void      setTrack(int val) override;
      virtual void      write(XmlWriter&) const override;
      virtual bool      readProperties(XmlReader&) override;
      virtual QString   accessibleInfo() const override;

      // properties
      QVariant getProperty(Pid ) const;
      bool setProperty(Pid propertyId, const QVariant&);
      QVariant propertyDefault(Pid id) const;

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;
      };


}     // namespace Ms
#endif

