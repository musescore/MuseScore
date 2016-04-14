#include <QMainWindow>
#include <QWidget>
#include "scoreaccessibility.h"
#include "musescore.h"
#include "libmscore/segment.h"
#include "libmscore/timesig.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "inspector/inspector.h"
#include "selectionwindow.h"
#include "playpanel.h"
#include "synthcontrol.h"
#include "mixer.h"
#include "drumroll.h"
#include "pianoroll.h"

namespace Ms{

AccessibleScoreView::AccessibleScoreView(ScoreView* scView) : QAccessibleWidget(scView){
      s = scView;
      }

int AccessibleScoreView::childCount() const{
      return 0;
      }

QAccessibleInterface* AccessibleScoreView::child(int /*index*/) const{
      return 0;
      }
QAccessibleInterface* AccessibleScoreView::parent() const{
      return QAccessibleWidget::parent();
      }
QRect AccessibleScoreView::rect() const{
      return s->rect();
      }
QAccessible::Role AccessibleScoreView::role() const{
      return QAccessible::NoRole;
      }

QString AccessibleScoreView::text(QAccessible::Text t) const {
      switch (t) {
            case QAccessible::Name:
                  return tr("Score %1").arg(s->score()->fileInfo()->completeBaseName());
            case QAccessible::Value:
                  return s->score()->accessibleInfo();
            default:
                  return QString();
           }
      return QString();
      }

QWindow* AccessibleScoreView::window() const {
      return qApp->focusWindow();
      }

QAccessibleInterface* AccessibleScoreView::ScoreViewFactory(const QString &classname, QObject *object)
      {
          QAccessibleInterface *iface = 0;
          if (classname == QLatin1String("Ms::ScoreView") && object && object->isWidgetType()){
//                qDebug("Creating interface for ScoreView object");
                iface = static_cast<QAccessibleInterface*>(new AccessibleScoreView(static_cast<ScoreView*>(object)));
                }

          return iface;
      }


ScoreAccessibility* ScoreAccessibility::inst = 0;

ScoreAccessibility::ScoreAccessibility(QMainWindow* mainWindow) : QObject(mainWindow)
      {
      this->mainWindow = mainWindow;
      statusBarLabel = 0;
      }

void ScoreAccessibility::createInstance(QMainWindow* mainWindow)
      {
      if (!inst) {
            inst = new ScoreAccessibility(mainWindow);
            }
      }

ScoreAccessibility::~ScoreAccessibility()
      {
      }

void ScoreAccessibility::clearAccessibilityInfo()
      {
      if(statusBarLabel != 0) {
            mainWindow->statusBar()->removeWidget(statusBarLabel);
            delete statusBarLabel;
            statusBarLabel = 0;
            static_cast<MuseScore*>(mainWindow)->currentScoreView()->score()->setAccessibleInfo(tr("No selection"));
            }
      }

void ScoreAccessibility::currentInfoChanged()
      {
      clearAccessibilityInfo();
      statusBarLabel  = new QLabel(mainWindow->statusBar());
      ScoreView* scoreView =  static_cast<MuseScore*>(mainWindow)->currentScoreView();
      Score* score = scoreView->score();
      if (score->selection().isSingle()) {
            Element* e = score->selection().element();
            if (!e) {
                  return;
                  }
            Element* el = e->isSpannerSegment() ? static_cast<SpannerSegment*>(e)->spanner() : e;
            QString barsAndBeats = "";
            std::pair<int, float> bar_beat;
            if (el->isSpanner()){
                  Spanner* s = static_cast<Spanner*>(el);
                  bar_beat = barbeat(s->startSegment());
                  barsAndBeats += tr("Start Measure: %1; Start Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(bar_beat.second));
                  Segment* seg = s->endSegment();
                  if(!seg)
                        seg = score->lastSegment()->prev1MM(Segment::Type::ChordRest);

                  if (seg->tick() != score->lastSegment()->prev1MM(Segment::Type::ChordRest)->tick() &&
                      s->type() != Element::Type::SLUR                                               &&
                      s->type() != Element::Type::TIE                                                )
                        seg = seg->prev1MM(Segment::Type::ChordRest);

                  bar_beat = barbeat(seg);
                  barsAndBeats += "; " + tr("End Measure: %1; End Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(bar_beat.second));
                  }
            else {
                  std::pair<int, float>bar_beat = barbeat(el);
                  if (bar_beat.first) {
                        barsAndBeats += " " + tr("Measure: %1").arg(QString::number(bar_beat.first));
                        if (bar_beat.second)
                              barsAndBeats += "; " + tr("Beat: %1").arg(QString::number(bar_beat.second));
                        }
                  }

            QString rez = e->accessibleInfo();
            if (!barsAndBeats.isEmpty())
                  rez += "; " + barsAndBeats;

            QString staff = "";
            if (e->staffIdx() + 1) {
                  staff = tr("Staff %1").arg(QString::number(e->staffIdx() + 1));
                  rez = QString("%1; %2").arg(rez).arg(staff);
                  }

            statusBarLabel->setText(rez);
            QString screenReaderRez = QString("%1%2 %3 %4").arg(e->screenReaderInfo()).arg(barsAndBeats).arg(staff).arg(e->accessibleExtraInfo());
            score->setAccessibleInfo(screenReaderRez);
            }
      else if (score->selection().isRange()) {
            QString barsAndBeats = "";
            std::pair<int, float> bar_beat;

            bar_beat = barbeat(score->selection().startSegment());
            barsAndBeats += " " + tr("Start Measure: %1; Start Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(bar_beat.second));
            Segment* endSegment = score->selection().endSegment();

            if (!endSegment)
                  endSegment = score->lastSegment();
            else
                  endSegment = endSegment->prev1MM();

            bar_beat = barbeat(endSegment);
            barsAndBeats += " " + tr("End Measure: %1; End Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(bar_beat.second));
            statusBarLabel->setText(tr("Range Selection") + barsAndBeats);
            score->setAccessibleInfo(tr("Range Selection") + barsAndBeats);
            }
      else if (score->selection().isList()) {
            statusBarLabel->setText(tr("List Selection"));
            score->setAccessibleInfo(tr("List Selection"));
            }
      mainWindow->statusBar()->addWidget(statusBarLabel);
      }

ScoreAccessibility* ScoreAccessibility::instance()
      {
      return inst;
      }

void ScoreAccessibility::updateAccessibilityInfo()
      {
      ScoreView* w = static_cast<MuseScore*>(mainWindow)->currentScoreView();
      if (!w) return;

      currentInfoChanged();

      //getInspector->isAncestorOf is used so that inspector and search dialog don't loose focus
      //when this method is called
      //TODO: create a class to manage focus and replace this massive if
      if ( (qApp->focusWidget() != w) &&
           !mscore->inspector()->isAncestorOf(qApp->focusWidget()) &&
           !(mscore->searchDialog() && mscore->searchDialog()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->getSelectionWindow() && mscore->getSelectionWindow()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->getPlayPanel() && mscore->getPlayPanel()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->getSynthControl() && mscore->getSynthControl()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->getMixer() && mscore->getMixer()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->searchDialog() && mscore->searchDialog()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->getDrumrollEditor() && mscore->getDrumrollEditor()->isAncestorOf(qApp->focusWidget())) &&
           !(mscore->getPianorollEditor() && mscore->getPianorollEditor()->isAncestorOf(qApp->focusWidget()))) {
            mscore->activateWindow();
            w->setFocus();
            }
      QObject* obj = static_cast<QObject*>(w);
      QAccessibleValueChangeEvent ev(obj, w->score()->accessibleInfo());
      QAccessible::updateAccessibility(&ev);
      }

std::pair<int, float> ScoreAccessibility::barbeat(Element *e)
      {
      if (!e) {
            return std::pair<int, float>(0, 0);
            }

      int bar = 0;
      int beat = 0;
      int ticks = 0;
      TimeSigMap* tsm = e->score()->sigmap();
      Element* p = e;
      while(p && p->type() != Element::Type::SEGMENT && p->type() != Element::Type::MEASURE)
            p = p->parent();

      if (!p) {
            return std::pair<int, float>(0, 0);
            }
      else if (p->type() == Element::Type::SEGMENT) {
            Segment* seg = static_cast<Segment*>(p);
            tsm->tickValues(seg->tick(), &bar, &beat, &ticks);
            }
      else if (p->type() == Element::Type::MEASURE) {
            Measure* m = static_cast<Measure*>(p);
            bar = m->no();
            beat = -1;
            ticks = 0;
            }
      return pair<int,float>(bar + 1, beat + 1 + ticks / static_cast<float>(MScore::division));
      }
}
