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

#ifndef __JUMP_H__
#define __JUMP_H__

#include "text.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Jump
///    Jump label
//
//   @P jumpTo      QString
//   @P playUntil   QString
//   @P continueAt  QString
//   @P jumpType    Ms::Jump::Type (DC, DC_AL_FINE, DC_AL_CODA, DS_AL_CODA, DS_AL_FINE, DS, USER) (read only)
//---------------------------------------------------------


class Jump : public Text {
      Q_OBJECT

      Q_PROPERTY(QString jumpTo      READ jumpTo      WRITE undoSetJumpTo)
      Q_PROPERTY(QString playUntil   READ playUntil   WRITE undoSetPlayUntil)
      Q_PROPERTY(QString continueAt  READ continueAt  WRITE undoSetContinueAt)
      //Q_Property(Ms::Jump::Type      READ jumpType)
      //Q_ENUMS(Type)

      QString _jumpTo;
      QString _playUntil;
      QString _continueAt;

   public:
      enum class Type : char {
            DC,
            DC_AL_FINE,
            DC_AL_CODA,
            DS_AL_CODA,
            DS_AL_FINE,
            DS,
            USER
            };

      Jump(Score*);

      void setJumpType(Type t);
      Type jumpType() const;
      QString jumpTypeUserName() const;

      virtual Jump* clone()          const { return new Jump(*this); }
      virtual Element::Type type()   const { return Element::Type::JUMP; }

      Measure* measure() const         { return (Measure*)parent(); }

      virtual void read(XmlReader&);
      virtual void write(Xml& xml)   const;

      QString jumpTo()               const { return _jumpTo;     }
      QString playUntil()            const { return _playUntil;  }
      QString continueAt()           const { return _continueAt; }
      void setJumpTo(const QString& s)     { _jumpTo = s;        }
      void setPlayUntil(const QString& s)  { _playUntil = s;     }
      void setContinueAt(const QString& s) { _continueAt = s;    }
      void undoSetJumpTo(const QString& s);
      void undoSetPlayUntil(const QString& s);
      void undoSetContinueAt(const QString& s);

      virtual bool systemFlag() const      { return true;        }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;

      Element* nextElement() override;
      Element* prevElement() override;
      virtual QString accessibleInfo() override;
      };


struct JumpTypeTable {
      Jump::Type type;
      TextStyleType textStyleType;
      const char* text;
      const char* jumpTo;
      const char* playUntil;
      const char* continueAt;
      QString userText;
      };

extern const JumpTypeTable jumpTypeTable[];
int jumpTypeTableSize();

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Jump::Type);

#endif

