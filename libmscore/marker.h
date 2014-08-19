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
//   @P label       QString
//   @P markerType  Ms::Marker::Type  (SEGNO, VARSEGNO, CODA, VARCODA, CODETTA, FINE, TOCODA, USER)
//---------------------------------------------------------

class Marker : public Text {
      Q_OBJECT

      Q_PROPERTY(QString label               READ label      WRITE undoSetLabel)
      Q_PROPERTY(Ms::Marker::Type markerType READ markerType WRITE undoSetMarkerType)
      Q_ENUMS(Type)

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

      void setMarkerType(Type t);
      Type markerType() const          { return _markerType; }
      QString markerTypeUserName();

      virtual Marker* clone() const override      { return new Marker(*this); }
      virtual Element::Type type() const override { return Element::Type::MARKER; }

      Measure* measure() const         { return (Measure*)parent(); }

      virtual void layout() override;
      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;

      QString label() const            { return _label; }
      void setLabel(const QString& s)  { _label = s; }
      void undoSetLabel(const QString& s);
      void undoSetMarkerType(Type t);

      virtual void styleChanged() override;
      virtual bool systemFlag() const override  { return true;        }
      virtual void adjustReadPos() override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      virtual QString accessibleInfo() override;
      };

typedef struct {
      Marker::Type type;
      QString name;
      } MarkerTypeItem;

extern const MarkerTypeItem markerTypeTable[];
int markerTypeTableSize();
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Marker::Type);

#endif

