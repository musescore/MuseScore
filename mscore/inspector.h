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

#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include "inspectorBase.h"
#include "ui_inspector_element.h"
#include "ui_inspector_vbox.h"
#include "ui_inspector_hbox.h"
#include "ui_inspector_articulation.h"
#include "ui_inspector_spacer.h"
#include "ui_inspector_segment.h"
#include "ui_inspector_note.h"
#include "ui_inspector_chord.h"
#include "ui_inspector_rest.h"
#include "ui_inspector_clef.h"
#include "ui_inspector_timesig.h"
#include "ui_inspector_keysig.h"
#include "ui_inspector_volta.h"
#include "ui_inspector_barline.h"

class Element;
class Note;
class Inspector;
class Segment;
class Chord;

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

class InspectorElement : public InspectorBase {
      Q_OBJECT
      Ui::InspectorElement b;

   public:
      InspectorElement(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

class InspectorVBox : public InspectorBase {
      Q_OBJECT
      Ui::InspectorVBox vb;

   public:
      InspectorVBox(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

class InspectorHBox : public InspectorBase {
      Q_OBJECT
      Ui::InspectorHBox hb;

   public:
      InspectorHBox(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

class InspectorArticulation : public InspectorBase {
      Q_OBJECT
      Ui::InspectorElement e;
      Ui::InspectorArticulation ar;

   public:
      InspectorArticulation(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

class InspectorSpacer : public InspectorBase {
      Q_OBJECT
      Ui::InspectorSpacer sp;

   public:
      InspectorSpacer(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

class InspectorRest : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorSegment s;
      Ui::InspectorRest    r;

   public:
      InspectorRest(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

class InspectorClef : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorSegment s;
      Ui::InspectorClef    c;

   public:
      InspectorClef(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

class InspectorTimeSig : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorSegment s;
      Ui::InspectorTimeSig t;

   public:
      InspectorTimeSig(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorKeySig
//---------------------------------------------------------

class InspectorKeySig : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorSegment s;
      Ui::InspectorKeySig k;

   public:
      InspectorKeySig(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

#define BARLINE_BUILTIN_SPANS 5

class InspectorBarLine : public InspectorBase {
      Q_OBJECT

      Ui::InspectorElement e;
      Ui::InspectorBarLine b;

      static QString builtinSpanNames[BARLINE_BUILTIN_SPANS];
      static int     builtinSpans[BARLINE_BUILTIN_SPANS][3];

//   public slots:
//      virtual void apply();

   public:
      InspectorBarLine(QWidget* parent);
//      virtual void setElement();
      };

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

class Inspector : public QDockWidget {
      Q_OBJECT

      QScrollArea* sa;
      InspectorBase* ie;
      QList<Element*> _el;
      Element* _element;      // currently displayed element
      bool _inspectorEdit;    // set to true when an edit originates from
                              // within the inspector itself

      virtual void closeEvent(QCloseEvent*);

   signals:
      void inspectorVisible(bool);

   public slots:
      void reset();

   public:
      Inspector(QWidget* parent = 0);
      void setElement(Element*);
      void setElements(const QList<Element*>&);
      Element* element() const            { return _element;       }
      const QList<Element*>& el() const   { return _el;            }
      bool inspectorEdit() const          { return _inspectorEdit; }
      void setInspectorEdit(bool val)     { _inspectorEdit = val;  }
      };

#endif

