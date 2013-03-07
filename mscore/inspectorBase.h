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
      P_ID t;           // property id
      int sv;           // subvalue; example for T_SIZE: 0 - width 1 - height
      int parent;       // apply to parent() element level
      QWidget* w;
      QToolButton* r;   // reset to default button (if any)
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
      QVector<InspectorItem> iList;
      QVBoxLayout* _layout;
      Inspector* inspector;

      virtual void setValue(const InspectorItem&, const QVariant&);
      QVariant getValue(const InspectorItem&) const;
      bool isDefault(const InspectorItem&);
      void mapSignals();

   public:
      InspectorBase(QWidget* parent);
      virtual void setElement();
      QWidget* addWidget();
      };

#endif

