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
//   InspectorPanel
//---------------------------------------------------------

struct InspectorPanel {
      QToolButton* title;
      QWidget* panel;
      };

//---------------------------------------------------------
//   InspectorItem
//---------------------------------------------------------

struct InspectorItem {
      Pid t;           // property id
      int parent;       // apply to parent() element level
      QWidget* w;
      QToolButton* r;   // reset to default button (if any)
      };

//---------------------------------------------------------
//   InspectorBase
//---------------------------------------------------------

class InspectorBase : public QWidget {
      Q_OBJECT

      bool dirty() const;
      void checkDifferentValues(const InspectorItem&);
      bool compareValues(const InspectorItem& ii, QVariant a, QVariant b);
      Element* effectiveElement(const InspectorItem&) const;

   private slots:
      void resetToStyle();

   protected slots:
      virtual void valueChanged(int idx, bool reset);
      virtual void valueChanged(int idx);
      void resetClicked(int);
      void setStyleClicked(int);

   protected:
      std::vector<InspectorItem> iList;
      std::vector<InspectorPanel> pList;
      QVBoxLayout* _layout;
      Inspector* inspector;

      virtual void setValue(const InspectorItem&, QVariant);
      QVariant getValue(const InspectorItem&) const;
      bool isDefault(const InspectorItem&);
      void mapSignals(const std::vector<InspectorItem>& il = std::vector<InspectorItem>(), const std::vector<InspectorPanel>& pl = std::vector<InspectorPanel>());
      void setupLineStyle(QComboBox*);

   public:
      InspectorBase(QWidget* parent);
      virtual void setElement();
      virtual void postInit() {} // called in setElement and valueChanged
      QWidget* addWidget();
      };


} // namespace Ms
#endif

