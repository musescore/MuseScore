#ifndef TIMELINEMETA_H
#define TIMELINEMETA_H

#include "timeline.h"
#include "timelineabstract.h"

namespace Ms {

class TimelineComponent;
class Timeline;
class TimelineMeta;
class TimelineMetaRows;
class TimelineMetaLabels;

//---------------------------------------------------------
//   TimelineMetaLabel
//---------------------------------------------------------

class TimelineMetaLabel : public TimelineLabel
      {
      TimelineMetaLabels* _parent;

   public:
      TimelineMetaLabel(TimelineMetaLabels* view, QString text, int nMeta, QFont font);
      };

//---------------------------------------------------------
//   TimelineMetaLabels
//---------------------------------------------------------

class TimelineMetaLabels : public QGraphicsView, public TimelineComponent
      {
      Q_OBJECT

      QList<TimelineMetaLabel*> _labels;
      int _maxTextWidth = 0;

   public slots:
      void updateLabelWidths(int newWidth);

   public:
      TimelineMetaLabels(TimelineMeta* parent);
      TimelineMeta* getParent();
      void updateLabels();
      int maximumTextWidth() { return _maxTextWidth; }
      };

//---------------------------------------------------------
//   Meta
//---------------------------------------------------------

struct Meta {
      QString metaName;
      bool visible;
      int order;
      Meta(QString name, int o) : metaName(name), visible(true), order(o) {}
      };

//---------------------------------------------------------
//   TimelineMetaRowsValue
//---------------------------------------------------------

class TimelineMetaRowsValue : public QGraphicsItemGroup
      {
      TimelineMetaRows* _parent;
      int _originalZValue;
      bool _selected = false;
      Element* _item;

      QGraphicsRectItem* _rectItem = nullptr;
      QGraphicsTextItem* _textItem = nullptr;

      // Used for redraw
      int _column;
      int _stagger;

      void positionText();
      qreal getTextWidth(QString text, QFont font);

   public:
      TimelineMetaRowsValue(TimelineMetaRows* parent, Element* element, QString text, int x, int stagger, int y, QFont font);
      void setZ(int z);
      void resetZ() { setZValue(_originalZValue); }
      int getZ() { return _originalZValue; }

      void resetBrush();

      void selectValue();
      void selectBrush();
      void hoverBrush();
      void deselect() { _selected = false; resetBrush(); }
      bool selected() { return _selected; }

      bool contains(Element* element);

      void redraw(int newWidth);
      };

//---------------------------------------------------------
//   TimelineMetaRows
//---------------------------------------------------------

class TimelineMetaRows : public QGraphicsView, public TimelineComponent
      {
      Q_OBJECT

      int _globalZValue; // Holds the z value for each new meta row
      QList<int> _staggerArray;
      const int _staggerDistance = 5;

      QPointF _oldMousePos;
      bool _draggingRows;

      TimelineMetaRowsValue* _oldHoverValue = nullptr;
      QFont _currentFont;

      QList<QGraphicsItem*> _redrawList;

      QList<TimelineMetaRowsValue*> _metaList;
      QList<TimelineMetaRowsValue*> getSelectedValues();

      void drawRows();
      void drawMeasureNumbers(int y);
      int getMeasureIncrement();
      void updateMetas();

      void drawMetaValue(Element* element, QString text, int x, int row);
      int getNewZValue() { return _globalZValue++; }
      int getStagger(int x);
      void resetStagger();

      void drawTempoMeta(std::vector<Element*> elist, int x, int row);
      void drawTimeSigMeta(Segment* segment, int x, int row);
      void drawRehersalMarkMeta(std::vector<Element*> elist, int x, int row);
      void drawKeySigMeta(Segment* segment, int x, int row);
      QString getKeyText(Key key);
      void drawBarlineMeta(Segment* segment, int x, int row);
      void drawJumpMarkersMeta(std::vector<Element*> elist, int x, int row);

      TimelineMetaRowsValue* getTopItem(QList<QGraphicsItem*> items);
      void selectValue(TimelineMetaRowsValue* value);
      void bringToFront(TimelineMetaRowsValue* value);
      void resetOldHover();

   public:
      TimelineMetaRows(TimelineMeta* parent);
      void updateRows();
      void updateSelection();
      void redrawRows();

      TimelineMeta* getParent();

      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      };

//---------------------------------------------------------
//   TimelineMeta
//---------------------------------------------------------

class TimelineMeta : public QSplitter, public TimelineComponent
      {
      Q_OBJECT

      QList<Meta> _metas;
      bool _collapsed = false;

      void updateScore();
      void updateScoreView();

   public slots:
      void dataSplitterMoved();

   public:
      TimelineMeta(Timeline* parent);
      void updateMeta();

      Timeline* getParent();

      TimelineMetaLabels* labelView() { return (TimelineMetaLabels*) widget(0); }
      TimelineMetaRows* rowsView() { return (TimelineMetaRows*) widget(1); }

      QList<int> getCorrectMetaRows();
      int nVisibleMetaRows();
      bool collapsed() { return _collapsed; }
      QList<Meta> metas() { return _metas; }

      enum class MetaRow {
            TEMPO,
            TIME_SIGNATURE,
            REHEARSAL_MARK,
            KEY_SIGNATURE,
            BARLINE,
            JUMPS_AND_MARKERS
            };
      };
}

#endif // TIMELINEMETA_H
