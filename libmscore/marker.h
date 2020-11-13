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
//   @@ Marker
//
//   @P label       string
//   @P markerType  enum (Marker.CODA, .CODETTA, .FINE, .SEGNO, .TOCODA, .USER, .VARCODA, .VARSEGNO)
//---------------------------------------------------------

class Marker final : public TextBase {
   public:
      enum class Type : char {
            SEGNO,
            VARSEGNO,
            CODA,
            VARCODA,
            CODETTA, // not in SMuFL, but still needed for 1.x compatibility, rendered as a double coda
            FINE,
            TOCODA,
            TOCODASYM,
            USER
            };

   private:
      Type _markerType;
      QString _label;               ///< referenced from Jump() element

   public:
      Marker(Score*);
      Marker(Score*, Tid);

      void setMarkerType(Type t);
      Type markerType() const          { return _markerType; }
      QString markerTypeUserName() const;
      Type markerType(const QString&) const;

      Marker* clone() const override    { return new Marker(*this); }
      ElementType type() const override { return ElementType::MARKER; }
      int subtype() const override      { return int(_markerType); }

      Measure* measure() const         { return (Measure*)parent(); }

      void layout() override;
      void read(XmlReader&) override;
      void write(XmlWriter& xml) const override;

      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }
      void undoSetLabel(const QString& s);
      void undoSetMarkerType(Type t);

      void styleChanged() override;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      Element* nextSegmentElement() override;
      Element* prevSegmentElement() override;
      QString accessibleInfo() const override;
      };

//---------------------------------------------------------
//   MarkerTypeItem
//---------------------------------------------------------

struct MarkerTypeItem {
      Marker::Type type;
      QString name;
      };

extern const MarkerTypeItem markerTypeTable[];
int markerTypeTableSize();

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Marker::Type);

#endif

