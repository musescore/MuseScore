//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __RANGE_H__
#define __RANGE_H__

#include "element.h"
#include "note.h"

class QPainter;

namespace Ms {

//---------------------------------------------------------
//   @@ Range
//---------------------------------------------------------

class Range : public Element {
      Q_OBJECT

      Note::NoteHeadGroup     _noteHeadGroup;
      Note::NoteHeadType      _noteHeadType;
      MScore::DirectionH      _dir;
      bool  _hasLine;
      qreal _lineWidth;                     // in spatium
      int   _topPitch, _bottomPitch;
      int   _topTpc, _bottomTpc;

      // internally managed, to optimize layout / drawing
      QPointF _topPos;                          // position of top note symbol
      QPointF _bottomPos;                       // position of bottom note symbol
      QLineF  _line;                            // the drawn line

      void  normalize();

   public:

      Range(Score* s);
      virtual Range* clone() const                    { return new Range(*this); }

      // getters and setters
      virtual ElementType type() const                { return RANGE;         }
      Note::NoteHeadGroup noteHeadGroup() const       { return _noteHeadGroup;}
      Note::NoteHeadType noteHeadType() const         { return _noteHeadType; }
      MScore::DirectionH direction() const            { return _dir;          }
      bool hasLine() const                            { return _hasLine;      }
      qreal lineWidth() const                         { return _lineWidth;    }
      int topOctave() const                           { return _topPitch / 12;}
      int bottomOctave() const                        { return _bottomPitch / 12;}
      int topPitch() const                            { return _topPitch;     }
      int bottomPitch() const                         { return _bottomPitch;  }
      int topTpc() const                              { return _topTpc;       }
      int bottomTpc() const                           { return _bottomTpc;    }

      void setNoteHeadGroup(Note::NoteHeadGroup val)  { _noteHeadGroup = val; }
      void setNoteHeadType (Note::NoteHeadType val)   { _noteHeadType  = val; }
      void setDirection    (MScore::DirectionH val)   { _dir = val;           }
      void setHasLine      (bool val)                 { _hasLine = val;       }
      void setLineWidth    (qreal val)                { _lineWidth = val;     }
      void setTopPitch     (int val);
      void setBottomPitch  (int val);
      void setTopTpc       (int val);
      void setBottomTpc    (int val);

      // some utility functions
      Segment* segment() const                        { return (Segment*)parent(); }
      int noteHead() const;
      qreal headWidth() const;
      void  updateRange();                // scan staff up to next section break and update range pitches

      // re-implemented virtual functions
      virtual Space space() const;
      virtual void draw(QPainter*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      virtual QPointF pagePos() const;      ///< position in page coordinates
      virtual void setTrack(int val);

      // properties
      QVariant getProperty(P_ID ) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;

      };


}     // namespace Ms
#endif

