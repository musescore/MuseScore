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

#ifndef __MARKER_H__
#define __MARKER_H__

#include "text.h"

namespace Ms {

//---------------------------------------------------------
//   MarkerType
//---------------------------------------------------------

enum class MarkerType {
      SEGNO,
      VARSEGNO,
      CODA,
      VARCODA,
      CODETTA,
      FINE,
      TOCODA,
      USER
      };

//---------------------------------------------------------
//   @@ Marker
//
//   @P label           QString
//   @P markerType      MArkerType
//---------------------------------------------------------

class Marker : public Text {
      Q_OBJECT

      Q_PROPERTY(QString label         READ label      WRITE undoSetLabel)
      Q_PROPERTY(MarkerType markerType READ markerType WRITE undoSetMarkerType)

      MarkerType _markerType;
      QString _label;               ///< referenced from Jump() element

      MarkerType markerType(const QString&) const;

   public:
      Marker(Score*);

      void setMarkerType(MarkerType t);
      MarkerType markerType() const    { return _markerType; }

      virtual Marker* clone() const    { return new Marker(*this); }
      virtual ElementType type() const { return MARKER; }

      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual void read(XmlReader&);
      virtual void write(Xml& xml) const;

      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }
      void undoSetLabel(const QString& s);
      void undoSetMarkerType(MarkerType t);

      virtual void styleChanged();
      virtual bool systemFlag() const  { return true;        }
      virtual void adjustReadPos();

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::MarkerType)

#endif

