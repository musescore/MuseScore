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
            CODETTA,
            FINE,
            TOCODA,
            USER
            };

   private:
      Type _markerType;
      QString _label;               ///< referenced from Jump() element

      Type markerType(const QString&) const;

   public:
      Marker(Score*);
      Marker(SubStyleId, Score*);

      void setMarkerType(Type t);
      Type markerType() const          { return _markerType; }
      QString markerTypeUserName() const;

      virtual Marker* clone() const override    { return new Marker(*this); }
      virtual ElementType type() const override { return ElementType::MARKER; }

      Measure* measure() const         { return (Measure*)parent(); }

      virtual void layout() override;
      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;

      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }
      void undoSetLabel(const QString& s);
      void undoSetMarkerType(Type t);

      virtual void styleChanged() override;
      virtual bool systemFlag() const override  { return true;        }
      virtual void adjustReadPos() override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;
      virtual QString accessibleInfo() const override;
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

