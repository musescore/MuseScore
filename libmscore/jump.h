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
//   JumpType
//---------------------------------------------------------

enum class JumpType : char {
      DC,
      DC_AL_FINE,
      DC_AL_CODA,
      DS_AL_CODA,
      DS_AL_FINE,
      DS,
      USER
      };

//---------------------------------------------------------
//   @@ Jump
///    Jump label
//
//   @P jumpTo      QString
//   @P playUntil   QString
//   @P continueAt  QString
//---------------------------------------------------------

class Jump : public Text {
      Q_OBJECT

      Q_PROPERTY(QString jumpTo     READ jumpTo     WRITE undoSetJumpTo)
      Q_PROPERTY(QString playUntil  READ playUntil  WRITE undoSetPlayUntil)
      Q_PROPERTY(QString continueAt READ continueAt WRITE undoSetContinueAt)

      QString _jumpTo;
      QString _playUntil;
      QString _continueAt;

   public:
      Jump(Score*);

      void setJumpType(JumpType t);
      JumpType jumpType() const;

      virtual Jump* clone()          const { return new Jump(*this); }
      virtual Element::Type type()   const { return Element::Type::JUMP; }

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
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::JumpType);

#endif

