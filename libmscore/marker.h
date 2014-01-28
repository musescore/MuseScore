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
//   @P markerType      MarkerType
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

      virtual Marker* clone() const override    { return new Marker(*this); }
      virtual ElementType type() const override { return MARKER; }

      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;

      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }
      void undoSetLabel(const QString& s);
      void undoSetMarkerType(MarkerType t);

      virtual void styleChanged() override;
      virtual bool systemFlag() const override  { return true;        }
      virtual void adjustReadPos() override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::MarkerType)

#endif

