//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "inspectorElementBase.h"
#include "inspectorTextBase.h"
#include "ui_inspector_break.h"
#include "ui_inspector_sectionbreak.h"
#include "ui_inspector_stafftypechange.h"
#include "ui_inspector_vbox.h"
#include "ui_inspector_tbox.h"
#include "ui_inspector_hbox.h"
#include "ui_inspector_articulation.h"
#include "ui_inspector_spacer.h"
#include "ui_inspector_segment.h"
#include "ui_inspector_note.h"
#include "ui_inspector_chord.h"
#include "ui_inspector_rest.h"
#include "ui_inspector_mmrest.h"
#include "ui_inspector_clef.h"
#include "ui_inspector_timesig.h"
#include "ui_inspector_keysig.h"
#include "ui_inspector_volta.h"
#include "ui_inspector_tuplet.h"
#include "ui_inspector_accidental.h"
#include "ui_inspector_tempotext.h"
#include "ui_inspector_lyric.h"
#include "ui_inspector_instrchange.h"
#include "ui_inspector_stafftext.h"
#include "ui_inspector_slur.h"
#include "ui_inspector_empty.h"
#include "ui_inspector_text.h"
// #include "ui_inspector_fret.h"
#include "ui_inspector_tremolo.h"
#include "ui_inspector_caesura.h"
#include "ui_inspector_bracket.h"
#include "ui_inspector_iname.h"
#include "ui_inspector_fermata.h"
#include "ui_inspector_stem.h"

namespace Ms {

class Score;
class Element;
class Note;
class Inspector;
class Segment;
class Chord;
class Clef;

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
//   InspectorSectionBreak
//---------------------------------------------------------

class InspectorSectionBreak : public InspectorBase {
      Q_OBJECT
      Ui::InspectorSectionBreak scb;

   public:
      InspectorSectionBreak(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorStaffTypeChange
//---------------------------------------------------------

class InspectorStaffTypeChange : public InspectorBase {
      Q_OBJECT
      Ui::InspectorStaffTypeChange sl;

   public:
      InspectorStaffTypeChange(QWidget* parent);
      virtual void setElement() override;
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

   private slots:
      void propertiesClicked();

   public:
      InspectorArticulation(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorFermata
//---------------------------------------------------------

class InspectorFermata : public InspectorElementBase {
      Q_OBJECT
      Ui::InspectorFermata f;

   public:
      InspectorFermata(QWidget* parent);
      virtual void setElement() override;
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

      QPushButton* dot1;
      QPushButton* dot2;
      QPushButton* dot3;
      QPushButton* dot4;
      QPushButton* tuplet;

      void dotClicked(int n);

   private slots:
      void dot1Clicked();
      void dot2Clicked();
      void dot3Clicked();
      void dot4Clicked();
      void tupletClicked();

   public:
      InspectorRest(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorMMRest
//---------------------------------------------------------

class InspectorMMRest : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorMMRest m;

   public:
      InspectorMMRest(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

class InspectorClef : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorClef    c;

   public:
      InspectorClef(QWidget* parent);
//      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorStem
//---------------------------------------------------------

class InspectorStem : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorStem s;

   public:
      InspectorStem(QWidget* parent);
//      virtual void setElement() override;
      };

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

class InspectorTimeSig : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSegment s;
      Ui::InspectorTimeSig t;

   private slots:
      void propertiesClicked();

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
//   InspectorTremolo
//---------------------------------------------------------

class InspectorTremolo : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorTremolo g;

   public:
      InspectorTremolo(QWidget* parent);
#if 0 // not needed currently
      virtual void setElement() override;
#endif
      };

//---------------------------------------------------------
//   InspectorTempoText
//---------------------------------------------------------

class InspectorTempoText : public InspectorTextBase {
      Q_OBJECT

      Ui::InspectorTempoText tt;

   public:
      InspectorTempoText(QWidget* parent);
      virtual void postInit() override;
      };

//---------------------------------------------------------
//   InspectorLyric
//---------------------------------------------------------

class InspectorLyric : public InspectorTextBase {
      Q_OBJECT

      Ui::InspectorLyric l;

   private slots:

   public:
      InspectorLyric(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorStaffText
//---------------------------------------------------------

class InspectorStaffText : public InspectorTextBase {
      Q_OBJECT

      Ui::InspectorStaffText s;

   private slots:
      void propertiesClicked();

   public:
      InspectorStaffText(QWidget* parent);
      virtual void setElement() override;
      };

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

class Inspector : public QDockWidget {
      Q_OBJECT

      QScrollArea* sa;
      InspectorBase* ie;
      Score* _score;
      bool _inspectorEdit;    // set to true when an edit originates from
                              // within the inspector itself
      Element* oe;
      bool oSameTypes;

   public slots:
      void update();

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate();

   public:
      Inspector(QWidget* parent = 0);
      void update(Score* s);

      Element* element() const;
      const QList<Element*>* el() const;
      void setInspectorEdit(bool val)     { _inspectorEdit = val;  }

      friend class InspectorScriptEntry;
      };

//---------------------------------------------------------
//   InspectorSlurTie
//---------------------------------------------------------

class InspectorSlurTie : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorSlur s;

   public:
      InspectorSlurTie(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorCaesura
//---------------------------------------------------------

class InspectorCaesura : public InspectorElementBase {
      Q_OBJECT

      Ui::InspectorCaesura c;

   public:
      InspectorCaesura(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorBracket
//---------------------------------------------------------

class InspectorBracket : public InspectorBase {
      Q_OBJECT

      Ui::InspectorBracket b;

   public:
      InspectorBracket(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorIname
//---------------------------------------------------------

class InspectorIname : public InspectorTextBase {
      Q_OBJECT

      Ui::InspectorIname i;

   public:
      InspectorIname(QWidget* parent);
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

extern void populatePlacement(QComboBox*);

} // namespace Ms
#endif

