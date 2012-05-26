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

class Element;
class Note;
class Inspector;
class Segment;
class Chord;

//---------------------------------------------------------
//   InspectorSegment
//---------------------------------------------------------

class InspectorSegment : public QWidget, Ui::InspectorSegment {
      Q_OBJECT
      Segment* segment;

   private slots:
      void resetLeadingSpaceClicked();
      void resetTrailingSpaceClicked();

      void leadingSpaceChanged(double);
      void trailingSpaceChanged(double);

   signals:
      void inspectorVisible(bool);
      void enableApply();

   public:
      InspectorSegment(QWidget* parent = 0);
      void setElement(Segment*);
      void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorChord
//---------------------------------------------------------

class InspectorChord : public QWidget, Ui::InspectorChord {
      Q_OBJECT
      Chord* chord;

   private slots:
      void smallChanged(bool val);
      void stemlessChanged(bool val);
      void stemDirectionChanged(int idx);

      void resetSmallClicked();
      void resetStemlessClicked();
      void resetStemDirectionClicked();

   signals:
      void inspectorVisible(bool);
      void enableApply();

   public:
      InspectorChord(QWidget* parent = 0);
      void setElement(Chord*);
      void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

class InspectorNoteBase : public QWidget, Ui::InspectorNote {
      Q_OBJECT
      Note* note;
      int _userVelocity;
      int _veloOffset;

      void block(bool);

   private slots:
      void resetSmallClicked();
      void resetMirrorClicked();
      void resetDotPositionClicked();
      void resetOntimeOffsetClicked();
      void resetOfftimeOffsetClicked();
      void resetNoteHeadGroupClicked();
      void resetNoteHeadTypeClicked();
      void resetTuningClicked();
      void resetVelocityTypeClicked();

      void smallChanged(int);
      void mirrorHeadChanged(int);
      void dotPositionChanged(int);
      void ontimeOffsetChanged(int);
      void offtimeOffsetChanged(int);

      void noteHeadGroupChanged(int);
      void noteHeadTypeChanged(int);
      void tuningChanged(double);
      void velocityTypeChanged(int);
      void velocityChanged(int);

   signals:
      void enableApply();

   public:
      InspectorNoteBase(QWidget* parent = 0);
      void setElement(Note*);
      void apply();
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorElementElement
//---------------------------------------------------------

class InspectorElementElement : public QWidget, Ui::InspectorElement {
      Q_OBJECT

      Element* e;

   private slots:
      void resetColorClicked();
      void resetXClicked();
      void resetYClicked();
      void colorChanged(QColor);
      void offsetXChanged(double);
      void offsetYChanged(double);
      void resetVisibleClicked();
      void apply();

   signals:
      void enableApply();

   public:
      InspectorElementElement(QWidget* parent = 0);
      void setElement(Element*);
      };

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

class InspectorElement : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* ie;

   public:
      InspectorElement(QWidget* parent);
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

class InspectorVBox : public InspectorBase {
      Q_OBJECT

      Ui::InspectorVBox vb;

      static const int _inspectorItems = 7;
      InspectorItem iList[_inspectorItems];

   protected:
      virtual const InspectorItem& item(int idx) const { return iList[idx]; }
      virtual int inspectorItems() const { return _inspectorItems; }

   public:
      InspectorVBox(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

class InspectorHBox : public InspectorBase {
      Q_OBJECT

      Ui::InspectorHBox hb;

      static const int _inspectorItems = 3;
      InspectorItem iList[_inspectorItems];

   protected:
      virtual const InspectorItem& item(int idx) const { return iList[idx]; }
      virtual int inspectorItems() const { return _inspectorItems; }

   public:
      InspectorHBox(QWidget* parent);
      };

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

class InspectorArticulation : public InspectorBase {
      Q_OBJECT

      Ui::InspectorArticulation ar;

   public slots:
      virtual void apply();

   public:
      InspectorArticulation(QWidget* parent);
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

class InspectorSpacer : public InspectorBase {
      Q_OBJECT

      Ui::InspectorSpacer sp;

   public slots:
      virtual void apply();

   public:
      InspectorSpacer(QWidget* parent);
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

class InspectorNote : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      InspectorNoteBase* iNote;
      InspectorChord*   iChord;
      InspectorSegment* iSegment;

      QToolButton* dot1;
      QToolButton* dot2;
      QToolButton* dot3;
      QToolButton* hook;
      QToolButton* stem;
      QToolButton* beam;

      bool dirty() const;

   private slots:
      void dot1Clicked();
      void dot2Clicked();
      void dot3Clicked();
      void hookClicked();
      void stemClicked();
      void beamClicked();

   public:
      InspectorNote(QWidget* parent);
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

class InspectorRest : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      InspectorSegment* iSegment;
      QCheckBox* small;

   public slots:
      virtual void apply();

   public:
      InspectorRest(QWidget* parent);
      virtual void setElement(Element*);
      bool dirty() const;
      };

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

class InspectorClef : public InspectorBase {
      Q_OBJECT

      InspectorElementElement* iElement;
      InspectorSegment* iSegment;

   public:
      InspectorClef(QWidget* parent);
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

class Inspector : public QDockWidget {
      Q_OBJECT

      QVBoxLayout* layout;
      InspectorBase* ie;
      Element* _element;
      QList<Element*> _el;

      virtual void closeEvent(QCloseEvent*);

   signals:
      void inspectorVisible(bool);

   public slots:
      void reset();

   public:
      Inspector(QWidget* parent = 0);
      void setElement(Element*);
      void setElementList(const QList<Element*>&);
      Element* element() const { return _element; }
      const QList<Element*>& el() const { return _el; }
      };

#endif

