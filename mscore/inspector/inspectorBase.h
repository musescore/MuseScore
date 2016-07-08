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
#include "libmscore/style.h"

namespace Ms {

class Inspector;
class Element;

//---------------------------------------------------------
//   InspectorItem
//---------------------------------------------------------

struct InspectorItem {
      P_ID t;           // property id
      int sv;           // subvalue; example for P_TYPE::SIZE: 0 - width 1 - height
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
      QSignalMapper* styleMapper;

      bool dirty() const;
      void checkDifferentValues(const InspectorItem&);

   protected slots:
      virtual void valueChanged(int idx, bool reset);
      virtual void valueChanged(int idx);
      void resetClicked(int);
      void setStyleClicked(int);

   protected:
      std::vector<InspectorItem> iList;
      QVBoxLayout* _layout;
      Inspector* inspector;

      virtual void setValue(const InspectorItem&, QVariant);
      QVariant getValue(const InspectorItem&) const;
      bool isDefault(const InspectorItem&);
      void mapSignals(const std::vector<InspectorItem>& il = std::vector<InspectorItem>());
      void setupLineStyle(QComboBox*);

   private slots:
      void resetToStyle();

   public:
      InspectorBase(QWidget* parent);
      virtual void setElement();
      virtual void postInit() {} // called in setElement and valueChanged
      QWidget* addWidget();
      };


} // namespace Ms
#endif

