//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __INSPECTOR_BASE_H__
#define __INSPECTOR_BASE_H__

#include "libmscore/property.h"

class Inspector;
class Element;

//---------------------------------------------------------
//   InspectorItem
//---------------------------------------------------------

struct InspectorItem {
      P_ID   t;
      int sv;           // subvalue; example for T_SIZE: 0 - width 1 - height
      QWidget* w;
      QToolButton* r;
      };

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

class InspectorBase : public QWidget {
      Q_OBJECT

      QSignalMapper* resetMapper;
      QSignalMapper* valueMapper;

      bool dirty() const;

   protected slots:
      virtual void valueChanged(int idx);
      void resetClicked(int);

   protected:
      QVBoxLayout* layout;
      Inspector* inspector;

      virtual const InspectorItem& item(int idx) const;
      virtual void setValue(int idx, const QVariant& val);

      QVariant getValue(int idx) const;

      bool isDefault(int idx);
      virtual int inspectorItems() const { return 0; }
      void mapSignals();

   public slots:
      virtual void apply();

   public:
      InspectorBase(QWidget* parent);
      virtual void setElement(Element*);
      };

#endif

