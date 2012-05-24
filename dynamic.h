//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: dynamic.h 5500 2012-03-28 16:28:26Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __DYNAMICS_H__
#define __DYNAMICS_H__

#include "text.h"
#include "mscore.h"

class Measure;
class Segment;

//---------------------------------------------------------
//   Dyn
//---------------------------------------------------------

struct Dyn {
      int velocity;           ///< associated midi velocity (0-127, -1 = none)
      bool accent;            ///< if true add velocity to current chord velocity
      const char* tag;

      Dyn(int velo, bool a, const char* t)
         : velocity(velo), accent(a), tag(t) {}
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

class Dynamic : public Text {
      int _subtype;

      mutable QPointF dragOffset;
      int _velocity;          // associated midi velocity 0-127
      DynamicType _dynType;

   public:
      Dynamic(Score*);
      Dynamic(const Dynamic&);
      virtual Dynamic* clone() const   { return new Dynamic(*this); }
      virtual ElementType type() const { return DYNAMIC; }
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      void setSubtype(int val);
      void setSubtype(const QString&);
      QString subtypeName() const;
      int subtype() const { return _subtype; }

      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void toDefault();

      void resetType();
      void setVelocity(int v);
      int velocity() const;
      DynamicType dynType() const    { return _dynType; }
      void setDynType(DynamicType t) { _dynType = t;    }


      virtual QLineF dragAnchor() const;
      };

extern Dyn dynList[];
#endif
