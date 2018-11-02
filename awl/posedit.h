//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __POSEDIT_H__
#define __POSEDIT_H__

#include "libmscore/pos.h"

namespace Awl {

//---------------------------------------------------------
//   PosEdit
//---------------------------------------------------------

class PosEdit : public QAbstractSpinBox
      {
      Q_OBJECT
      Q_PROPERTY(bool smpte READ smpte WRITE setSmpte)

      bool _smpte;
      Pos _pos;
      bool initialized;

      virtual void paintEvent(QPaintEvent* event);
      virtual void stepBy(int steps);
      virtual StepEnabled stepEnabled() const;
      virtual void fixup(QString& input) const;
      virtual QValidator::State validate(QString&, int&) const;
      void updateValue();
      int curSegment() const;
      virtual bool event(QEvent*);

   signals:
      void valueChanged(const Pos&);

   public slots:
      void setValue(const Pos& time);
      void setValue(int t);
      void setValue(const QString& s);

   public:
      PosEdit(QWidget* parent = 0);
      ~PosEdit();
      QSize sizeHint() const;

      Pos pos() const { return _pos; }
      void setSmpte(bool);
      bool smpte() const { return _smpte; }
      void* operator new(size_t);
      };
}

#endif
