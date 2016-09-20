//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: inspector.h
//
//  Copyright (C) 2011-2016 Werner Schweer and others
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
#include "ui_inspector_bend.h"
#include "ui_inspector_break.h"
#include "ui_inspector_vbox.h"
#include "ui_inspector_tbox.h"
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
#include "ui_inspector_tuplet.h"
#include "ui_inspector_accidental.h"
#include "ui_inspector_tempotext.h"
#include "ui_inspector_dynamic.h"
#include "ui_inspector_lyric.h"
#include "ui_inspector_stafftext.h"
#include "ui_inspector_slur.h"
#include "ui_inspector_empty.h"
#include "ui_inspector_text.h"
#include "ui_inspector_fret.h"
#include "ui_inspector_tremolo.h"
#include "ui_inspector_caesura.h"

namespace Ms {

class Element;
class Note;
class Inspector;
class Segment;
class Chord;
class Clef;

//---------------------------------------------------------
//   UiInspectorElement
//---------------------------------------------------------

class UiInspectorElement: public Ui::InspectorElement {
   public:
      void setupUi(QWidget *InspectorElement);
      };

//---------------------------------------------------------
//   InspectorElementBase
//---------------------------------------------------------

class InspectorElementBase : public InspectorBase {
      Q_OBJECT

   protected:
      UiInspectorElement e;

   private slots:
      void resetAutoplace();
      void autoplaceChanged(bool);

   public:
      InspectorElementBase(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

class InspectorElement : public InspectorElementBase {
      Q_OBJECT

   public:
      InspectorElement(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorBreak
//---------------------------------------------------------

class InspectorBreak : public InspectorBase {
      Q_OBJECT
      Ui::InspectorBreak b;

   public:
      InspectorBreak(QWidget* parent);
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
//   InspectorTBox
//---------------------------------------------------------

class InspectorTBox : public InspectorBase {
      Q_OBJECT
      Ui::InspectorTBox tb;

   public:
      InspectorTBox(QWidget* parent);
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

class InspectorArticulation : public InspectorElementBase {
      Q_OBJECT
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

class InspectorRest : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorRest    r;

      QToolButton* tuplet;

   private slots:
      void tupletClicked();

   public:
      InspectorRest(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------


class InspectorClef : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorClef    c;
      Clef* otherClef;        // the courtesy clef for a main clef or viceversa
                              // used to keep in sync ShowCourtesy setting of both clefs
   protected slots:
      virtual void valueChanged(int idx) override;

   public:
      InspectorClef(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

class InspectorTimeSig : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorTimeSig t;

   public:
      InspectorTimeSig(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorKeySig
//---------------------------------------------------------

class InspectorKeySig : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorKeySig k;

   public:
      InspectorKeySig(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorTuplet
//---------------------------------------------------------

class InspectorTuplet : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorTuplet t;

   public:
      InspectorTuplet(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorAccidental
//---------------------------------------------------------

class InspectorAccidental : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorAccidental a;

   public:
      InspectorAccidental(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorBend
//---------------------------------------------------------

class InspectorBend : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorBend g;

   private slots:
      void propertiesClicked();

   public:
      InspectorBend(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorTremoloBar
//---------------------------------------------------------

class InspectorTremoloBar : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorTremoloBar g;

   private slots:
      void propertiesClicked();

   public:
      InspectorTremoloBar(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorTempoText
//---------------------------------------------------------

class InspectorTempoText : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorText t;
      Ui::InspectorTempoText tt;

   public:
      InspectorTempoText(QWidget* parent);
      virtual void setElement() override;
      virtual void postInit() override;
      };

//---------------------------------------------------------
//   InspectorDynamic
//---------------------------------------------------------

class InspectorDynamic : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement e;
      Ui::InspectorText t;
      Ui::InspectorDynamic d;

   public:
      InspectorDynamic(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorLyric
//---------------------------------------------------------

class InspectorLyric : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement e;
      Ui::InspectorText t;
      Ui::InspectorLyric l;

   private slots:
      virtual void valueChanged(int idx) override;

   public:
      InspectorLyric(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorLyric
//---------------------------------------------------------

class InspectorStafftext : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement e;
      Ui::InspectorText t;
      Ui::InspectorStafftext s;

   public:
      InspectorStafftext(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

class InspectorBarLine : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorBarLine b;

      void  blockSpanDataSignals(bool val);

   private slots:
      void manageSpanData();
      void presetDefaultClicked();
      void presetTick1Clicked();
      void presetTick2Clicked();
      void presetShort1Clicked();
      void presetShort2Clicked();

   public:
      InspectorBarLine(QWidget* parent);
      virtual void setElement() override;
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

   public slots:
      void reset();

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate();

   public:
      Inspector(QWidget* parent = 0);
      void setElement(Element*);
      void setElements(const QList<Element*>&);
      Element* element() const            { return _element;       }
      const QList<Element*>& el() const   { return _el;            }
      void setInspectorEdit(bool val)     { _inspectorEdit = val;  }
      };

//---------------------------------------------------------
//   InspectorSlur
//---------------------------------------------------------

class InspectorSlur : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSlur s;

   public:
      InspectorSlur(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorCaesura
//---------------------------------------------------------

class InspectorCaesura : public InspectorElementBase {
      Q_OBJECT

//      UiInspectorElement e;
      Ui::InspectorCaesura c;

   public:
      InspectorCaesura(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorEmpty
//---------------------------------------------------------

class InspectorEmpty : public InspectorBase {
      Q_OBJECT

      Ui::InspectorEmpty e;

   public:
      InspectorEmpty(QWidget* parent);
      virtual QSize sizeHint() const override;
      };

} // namespace Ms
#endif

