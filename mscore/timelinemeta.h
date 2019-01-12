#ifndef TIMELINEMETA_H
#define TIMELINEMETA_H

#include "timeline.h"
#include "timelinelabel.h"

namespace Ms {

class Timeline;
//class TimelineLabel;
class TimelineMeta;
class TimelineMetaRows;
class TimelineMetaLabels;

class TimelineMetaLabel : public TimelineLabel
      {

      TimelineMetaLabels* _parent;

   public:
      TimelineMetaLabel(TimelineMetaLabels* view, QString text, int nMeta);
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

      void positionText(QGraphicsRectItem* rectItem, QGraphicsTextItem* textItem);
      qreal getTextWidth(QString text);
      QGraphicsRectItem* getRect();

   public:
      TimelineMetaRowsValue(TimelineMetaRows* parent, Element* element, QString text, int x, int y);
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
      };

class TimelineMetaLabels : public QGraphicsView
      {
      Q_OBJECT

      QList<TimelineMetaLabel*> _labels;

   public slots:
      void updateLabelWidths(int newWidth);

   public:
      TimelineMetaLabels(TimelineMeta* parent);
      TimelineMeta* getParent();
      void updateLabels();
      Score* score();
      };

//---------------------------------------------------------
//   TimelineMetaRows
//---------------------------------------------------------

class TimelineMetaRows : public QGraphicsView
      {
      Q_OBJECT

      int _globalZValue; // Holds the z value for each new meta row
      QList<int> _staggerArray;
      const int _staggerDistance = 5;

      TimelineMetaRowsValue* _oldHoverValue = nullptr;

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

      int cellWidth();
      int cellHeight();
      int minCellWidth();
      int maxCellWidth();

      TimelineMeta* getParent();
      Score* score();
      ScoreView* scoreView();

      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      };

//---------------------------------------------------------
//   TimelineMeta
//---------------------------------------------------------

class TimelineMeta : public QSplitter
      {
      Q_OBJECT

      QList<Meta> _metas;
      bool _collapsed = false;

   public slots:
      void dataSplitterMoved();

   public:
      TimelineMeta(Timeline* parent);
      void updateMeta();

      Timeline* getParent();
      Score* score();
      ScoreView* scoreView();
      int cellHeight();
      int cellWidth();

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
