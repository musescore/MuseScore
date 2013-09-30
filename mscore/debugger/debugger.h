//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: debugger.h 5383 2012-02-27 07:38:15Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include "ui_debugger.h"
#include "ui_element.h"
#include "ui_note.h"
#include "ui_page.h"
#include "ui_text.h"
#include "ui_measure.h"
#include "ui_segment.h"
#include "ui_chord.h"
#include "ui_chordrest.h"
#include "ui_hairpin.h"
#include "ui_barline.h"
#include "ui_dynamic.h"
#include "ui_tuplet.h"
#include "ui_slurtie.h"
#include "ui_tie.h"
#include "ui_line.h"
#include "ui_textline.h"
#include "ui_linesegment.h"
#include "ui_lyrics.h"
#include "ui_beam.h"
#include "ui_tremolo.h"
#include "ui_spanner.h"
#include "ui_slursegment.h"
#include "ui_accidental.h"
#include "ui_clef.h"
#include "ui_articulationbase.h"
#include "ui_keysig.h"
#include "ui_rest.h"
#include "ui_stem.h"
#include "ui_box.h"
#include "ui_harmony.h"
#include "ui_spanner.h"
#include "ui_system.h"

#include "globals.h"
#include "libmscore/element.h"
#include "libmscore/mscore.h"

namespace Ms {

class ShowElementBase;
class Element;
class Page;
class DoubleLabel;
class Score;
class BSymbol;
class ElementItem;

class ShowNoteWidget;

//---------------------------------------------------------
//   Debugger
//---------------------------------------------------------

class Debugger : public QDialog, public Ui::DebuggerBase {
      Q_OBJECT;

      QStack<Element*>backStack;
      QStack<Element*>forwardStack;

      ShowElementBase* elementViews[Element::MAXTYPE];

      bool searchElement(QTreeWidgetItem* pi, Element* el);
//      void addSymbol(ElementItem* parent, BSymbol* bs);
      void updateElement(Element*);
      virtual void showEvent(QShowEvent*);
      void addMeasure(ElementItem* mi, Measure* measure);

   protected:
      Score* cs;
      Element* curElement;

   private slots:
      void itemClicked(QTreeWidgetItem*, int);
      void itemExpanded(QTreeWidgetItem*);
      void layoutScore();
      void backClicked();
      void forwardClicked();
      void selectElement();
      void resetElement();
      void layout();

   public slots:
      void setElement(Element*);
      void reloadClicked();

   public:
      Debugger(QWidget* parent = 0);
      void writeSettings();
	void updateList(Score*);
      };

//---------------------------------------------------------
//   MeasureListEditor
//---------------------------------------------------------

class MeasureListEditor : public QWidget {
      Q_OBJECT;

   private slots:
      // void itemChanged(QListViewItem*);

   public:
      MeasureListEditor();
      };

//---------------------------------------------------------
//   ShowElementBase
//---------------------------------------------------------

class ShowElementBase : public QWidget {
      Q_OBJECT;

      Ui::ElementBase eb;
      Element* el;

   private slots:
      void parentClicked();
      void linkClicked();
      void offsetxChanged(double);
      void offsetyChanged(double);
      void selectedClicked(bool);
      void visibleClicked(bool);

   public slots:
      void gotoElement(QListWidgetItem*);
      void gotoElement(QTreeWidgetItem*);

   protected:
      QVBoxLayout* layout;

   signals:
      void elementChanged(Element*);

   public:
      ShowElementBase();
      virtual void setElement(Element*);
      Element* element() const { return el; }
      QWidget* addWidget();
      };

//---------------------------------------------------------
//   ShowPageWidget
//---------------------------------------------------------

class ShowPageWidget : public ShowElementBase {
      Q_OBJECT;

      Ui::PageBase pb;

   private slots:
      void itemClicked(QListWidgetItem*);

   public:
      ShowPageWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   MeasureView
//---------------------------------------------------------

class MeasureView : public ShowElementBase {
      Q_OBJECT;

      Ui::MeasureBase mb;

   private slots:
      void elementClicked(QTreeWidgetItem* item);
      void nextClicked();
      void prevClicked();
      void mmRestClicked();

   public:
      MeasureView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ChordDebug
//---------------------------------------------------------

class ChordDebug : public ShowElementBase {
      Q_OBJECT;
      Ui::ChordRestBase crb;
      Ui::ChordBase cb;

   private slots:
      void hookClicked();
      void stemClicked();
      void directionChanged(int);
      void beamClicked();
      void tupletClicked();
      void upChanged(bool);
      void beamModeChanged(int);
      void stemSlashClicked();
      void arpeggioClicked();
      void tremoloClicked();
      void glissandoClicked();

   public:
      ChordDebug();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   SegmentView
//---------------------------------------------------------

class SegmentView : public ShowElementBase {
      Q_OBJECT;
      Ui::SegmentBase sb;

   public:
      SegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowNoteWidget
//---------------------------------------------------------

class ShowNoteWidget : public ShowElementBase {
      Q_OBJECT;

      Ui::NoteBase nb;

   private slots:
      void tieForClicked();
      void tieBackClicked();
      void accidentalClicked();
      void tpcChanged(int);
      void dot1Clicked();
      void dot2Clicked();
      void dot3Clicked();

   signals:
      void scoreChanged();

   public:
      ShowNoteWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   RestView
//---------------------------------------------------------

class RestView : public ShowElementBase {
      Q_OBJECT;

      Ui::ChordRestBase crb;
      Ui::Rest rb;

   private slots:
      void tupletClicked();
      void beamClicked();

   public:
      RestView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ShowTimesigWidget
//---------------------------------------------------------

class ShowTimesigWidget : public ShowElementBase {
      Q_OBJECT;

   public:
      ShowTimesigWidget();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TextView
//---------------------------------------------------------

class TextView : public ShowElementBase {
      Q_OBJECT;

      Ui::TextBase tb;

   private slots:
      void textChanged();

   signals:
      void scoreChanged();

   public:
      TextView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   HarmonyView
//---------------------------------------------------------

class HarmonyView : public ShowElementBase {
      Q_OBJECT;

      Ui::TextBase tb;
      Ui::HarmonyBase hb;

   public:
      HarmonyView();
      virtual void setElement(Element*);
   private slots:
      void on_leftParen_clicked(bool checked);
      void on_rightParen_clicked(bool checked);
      };

//---------------------------------------------------------
//   SpannerView
//---------------------------------------------------------

class SpannerView : public ShowElementBase {
      Q_OBJECT;

      Ui::SpannerBase sp;

   private slots:
      void startClicked();
      void endClicked();

   public:
      SpannerView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   HairpinView
//---------------------------------------------------------

class HairpinView : public SpannerView {
      Q_OBJECT;

      Ui::HairpinBase hp;
      Ui::SLineBase sl;

   public:
      HairpinView();
      virtual void setElement(Element*);
      };



//---------------------------------------------------------
//   ElementView
//---------------------------------------------------------

class ElementView : public ShowElementBase {
      Q_OBJECT;

   public:
      ElementView();
      };

//---------------------------------------------------------
//   BarLineView
//---------------------------------------------------------

class BarLineView : public ShowElementBase {
      Q_OBJECT;

      Ui::BarLineBase bl;

   public:
      BarLineView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   DynamicView
//---------------------------------------------------------

class DynamicView : public ShowElementBase {
      Q_OBJECT;

      Ui::TextBase tb;
      Ui::DynamicBase bl;

   public:
      DynamicView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TupletView
//---------------------------------------------------------

class TupletView : public ShowElementBase {
      Q_OBJECT;

      Ui::TupletBase tb;

   signals:
      void itemClicked(Element*);
      void scoreChanged();

   private slots:
      void numberClicked();
      void elementClicked(QTreeWidgetItem*);

   public:
      TupletView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   DoubleLabel
//---------------------------------------------------------

class DoubleLabel : public QLabel {
      Q_OBJECT;

   public:
      DoubleLabel(QWidget* parent);
      void setValue(double);
      virtual QSize sizeHint() const;
      };

//---------------------------------------------------------
//   SlurTieView
//---------------------------------------------------------

class SlurTieView : public SpannerView {
      Q_OBJECT;

      Ui::SlurTieBase st;

   private slots:
      void segmentClicked(QTreeWidgetItem* item);

   public:
      SlurTieView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TieView
//---------------------------------------------------------

class TieView : public SlurTieView {
      Q_OBJECT;

      Ui::TieBase tb;

   private slots:
      void startClicked();
      void endClicked();

   public:
      TieView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   VoltaView
//---------------------------------------------------------

class VoltaView : public ShowElementBase {
      Q_OBJECT;

      Ui::TextLineBase tlb;
      Ui::SLineBase lb;
      Ui::SpannerBase sp;

   private slots:
      void segmentClicked(QTreeWidgetItem* item);
      void beginTextClicked();
      void continueTextClicked();
      void leftElementClicked();
      void rightElementClicked();
      void startClicked();
      void endClicked();

   public:
      VoltaView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

class VoltaSegmentView : public ShowElementBase {
      Q_OBJECT;

      Ui::LineSegmentBase lb;

   public:
      VoltaSegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TextLineView
//---------------------------------------------------------

class TextLineView : public SpannerView {
      Q_OBJECT;

      Ui::TextLineBase tlb;
      Ui::SLineBase lb;

   private slots:
      void beginTextClicked();
      void continueTextClicked();

   public:
      TextLineView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TextLineSegmentView
//---------------------------------------------------------

class TextLineSegmentView : public ShowElementBase {
      Q_OBJECT;

      Ui::LineSegmentBase lb;

   private slots:
      void lineClicked();

   public:
      TextLineSegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   LineSegmentView
//---------------------------------------------------------

class LineSegmentView : public ShowElementBase {
      Q_OBJECT;

      Ui::LineSegmentBase lb;

   public:
      LineSegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   VoltaSegmentView
//---------------------------------------------------------

class LyricsView : public ShowElementBase {
      Q_OBJECT;

      Ui::LyricsBase lb;

   public:
      LyricsView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   BeamView
//---------------------------------------------------------

class BeamView : public ShowElementBase {
      Q_OBJECT;

      Ui::BeamBase bb;

   private slots:
      void elementClicked(QTreeWidgetItem*);

   public:
      BeamView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   TremoloView
//---------------------------------------------------------

class TremoloView : public ShowElementBase {
      Q_OBJECT;

      Ui::TremoloBase tb;

   private slots:
      void chord1Clicked();
      void chord2Clicked();

   public:
      TremoloView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   OttavaView
//---------------------------------------------------------

class OttavaView : public TextLineView {
      Q_OBJECT;

//      Ui::OttavaBase ob;

   private slots:

   public:
      OttavaView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   SlurSegmentView
//---------------------------------------------------------

class SlurSegmentView : public ShowElementBase {
      Q_OBJECT;

      Ui::SlurSegment ss;

   private slots:
      void slurTieClicked();

   public:
      SlurSegmentView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   AccidentalView
//---------------------------------------------------------

class AccidentalView : public ShowElementBase {
      Q_OBJECT;

      Ui::Accidental acc;

   public:
      AccidentalView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ClefView
//---------------------------------------------------------

class ClefView : public ShowElementBase {
      Q_OBJECT;

      Ui::Clef clef;

   public:
      ClefView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   ArticulationView
//---------------------------------------------------------

class ArticulationView : public ShowElementBase {
      Q_OBJECT;

      Ui::ArticulationBase articulation;

   public:
      ArticulationView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   KeySigView
//---------------------------------------------------------

class KeySigView : public ShowElementBase {
      Q_OBJECT;

      Ui::KeySig keysig;

   public:
      KeySigView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   StemView
//---------------------------------------------------------

class StemView : public ShowElementBase {
      Q_OBJECT;

      Ui::StemBase stem;

   public:
      StemView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   BoxView
//---------------------------------------------------------

class BoxView : public ShowElementBase {
      Q_OBJECT;

      Ui::BoxBase box;

   public:
      BoxView();
      virtual void setElement(Element*);
      };

//---------------------------------------------------------
//   SystemView
//---------------------------------------------------------

class SystemView : public ShowElementBase {
      Q_OBJECT;

      Ui::SystemBase mb;

   private slots:
      void elementClicked(QTreeWidgetItem*);
      void measureClicked(QListWidgetItem*);

   public:
      SystemView();
      virtual void setElement(Element*);
      };

} // namespace Ms
#endif

