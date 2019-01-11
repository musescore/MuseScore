#include "timelinemeta.h"
#include "musescore.h"
#include "libmscore/barline.h"
#include "libmscore/tempotext.h"
#include "libmscore/timesig.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/staff.h"
#include "libmscore/keysig.h"
#include "libmscore/key.h"
#include "libmscore/interval.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"

namespace Ms {

TimelineMetaLabel::TimelineMetaLabel(TimelineMetaLabels *view, int nMeta, int height)
      : TimelineLabel(view, nMeta, height)
      {

      }

//---------------------------------------------------------
//   TimelineMetaRowsValue
//---------------------------------------------------------

TimelineMetaRowsValue::TimelineMetaRowsValue(TimelineMetaRows* parent, Element* element, QString text, int x, int y)
      : _parent(parent), _item(element)
      {
      const int margin = 4;
      QGraphicsRectItem* rectItem = new QGraphicsRectItem(x, y, getTextWidth(text) + margin, _parent->cellHeight());
      addToGroup(rectItem);

      QGraphicsTextItem* textItem = new QGraphicsTextItem(text, this);
      textItem->setFont(_parent->getParent()->getParent()->getFont());
      positionText(rectItem, textItem);
      addToGroup(textItem);

      resetBrush();
      }

//---------------------------------------------------------
//   positionText
//---------------------------------------------------------

void TimelineMetaRowsValue::positionText(QGraphicsRectItem* rectItem, QGraphicsTextItem* textItem)
      {
      QPointF targetCenter = rectItem->boundingRect().center();
      QPointF offset = targetCenter - textItem->boundingRect().center();
      textItem->setPos(textItem->boundingRect().translated(offset).topLeft());
      }

//---------------------------------------------------------
//   getTextWidth
//---------------------------------------------------------

qreal TimelineMetaRowsValue::getTextWidth(QString text)
      {
      // TODO: Elide text if it extends past grid width
      QFont applicationFont = _parent->getParent()->getParent()->getFont();
      QSizeF textSize = QFontMetricsF(applicationFont).size(Qt::TextSingleLine, text);
      return textSize.width();
      }

//---------------------------------------------------------
//   setZ
//---------------------------------------------------------

void TimelineMetaRowsValue::setZ(int z)
      {
      _originalZValue = z;
      setZValue(z);
      }

//---------------------------------------------------------
//   getRect
//---------------------------------------------------------

QGraphicsRectItem* TimelineMetaRowsValue::getRect()
      {
      QList<QGraphicsItem*> items = childItems();
      for (QGraphicsItem* item : items) {
            QGraphicsRectItem* rect = qgraphicsitem_cast<QGraphicsRectItem*>(item);
            if (rect)
                  return rect;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   resetBrush
//---------------------------------------------------------

void TimelineMetaRowsValue::resetBrush()
      {
      if (_selected)
            selectBrush();
      else
            getRect()->setBrush(QBrush(Qt::darkGray));
      }

//---------------------------------------------------------
//   selectValue
//---------------------------------------------------------

void TimelineMetaRowsValue::selectValue()
      {
      Score* score = _parent->score();
      score->deselectAll();

      if (_item) {
            if (_item->isSegment()) {
                  Segment* segment = toSegment(_item);
                  for (Element* element : segment->elist()) {
                        if (element)
                              score->select(element, SelectType::ADD);
                        }
                  }
            else
                  score->select(_item, SelectType::ADD);
            _parent->scoreView()->adjustCanvasPosition(_item->findMeasure(), false, _item->staffIdx());
            }

      mscore->endCmd(); // Also updates value colors
      }

//---------------------------------------------------------
//   selectBrush
//---------------------------------------------------------

void TimelineMetaRowsValue::selectBrush()
      {
      _selected = true;
      getRect()->setBrush(QBrush(Qt::blue));
      }

//---------------------------------------------------------
//   hoverBrush
//---------------------------------------------------------

void TimelineMetaRowsValue::hoverBrush()
      {
      if (!_selected)
            getRect()->setBrush(QBrush(Qt::red));
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool TimelineMetaRowsValue::contains(Element *element)
      {
      if (!_item)
            return false;

      if (_item == element)
            return true;

      if (_item->isSegment()) {
            Segment* segment = toSegment(_item);
            for (Element* el : segment->elist()) {
                  if (el == element)
                        return true;
                  }
            }

      return false;
      }

TimelineMetaLabels::TimelineMetaLabels(TimelineMeta* parent)
      : QGraphicsView(parent)
      {
      setScene(new QGraphicsScene);
      scene()->setBackgroundBrush(Qt::lightGray);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));
      connect(parent, SIGNAL(splitterMoved(int, int)), this, SLOT(updateLabelWidths(int)));

      qDebug() << parent->sizes();

      setSceneRect(-1, -1, parent->sizes()[0], parent->getParent()->cellHeight() * parent->nVisibleMetaRows());
      }

TimelineMeta* TimelineMetaLabels::getParent()
      {
      return static_cast<TimelineMeta*>(parent());
      }

void TimelineMetaLabels::updateLabelWidths(int newWidth)
      {
      for (TimelineMetaLabel* label : _labels)
            label->updateWidth(newWidth);

      // -1 makes sure the rect border is within view, -2 makes sure the scene rect is always smaller than the view rect
      setSceneRect(-1, -1, newWidth - 2, getParent()->getParent()->cellHeight() * _labels.length() + 1);
      }

void TimelineMetaLabels::updateLabels()
      {
      _labels.clear();
      scene()->clear();

      if (!score())
            return;

      int nMeta = 0;
      for (Meta meta : getParent()->metas()) {
            TimelineMetaLabel* metaLabel = new TimelineMetaLabel(this, nMeta, getParent()->getParent()->cellHeight());
            _labels.append(metaLabel);
            scene()->addItem(metaLabel);
            nMeta++;
            }
      // Add measure label here

      updateLabelWidths(getParent()->sizes()[0]);
      }

Score* TimelineMetaLabels::score()
      {
      return getParent()->score();
      }

//---------------------------------------------------------
//   TimelineMetaRows
//---------------------------------------------------------

TimelineMetaRows::TimelineMetaRows(TimelineMeta *parent)
      : QGraphicsView(parent)
      {
      setScene(new QGraphicsScene);
      scene()->setBackgroundBrush(Qt::lightGray);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));

      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      // What to do with vertical? If data is visible, turn it on?
      // Do we need scroll bars? (Yes)

      for (int i = 0; i < getParent()->getCorrectMetaRows().length(); i++)
            _staggerArray << 0;
      }

//---------------------------------------------------------
//   updateRows
//---------------------------------------------------------

void TimelineMetaRows::updateRows()
      {
      scene()->clear();
      _oldHoverValue = nullptr;

      drawRows();
      updateMetas();
      }

//---------------------------------------------------------
//   updateSelection
//---------------------------------------------------------

void TimelineMetaRows::updateSelection()
      {

      QList<TimelineMetaRowsValue*> values = getSelectedValues();

      for (TimelineMetaRowsValue* value : _metaList) {
            if (values.contains(value)) {
                  value->selectBrush();
                  }
            else {
                  value->deselect();
                  }
            }
      }

//---------------------------------------------------------
//   getSelectedValues
// TODO: Improve search algo
//---------------------------------------------------------

QList<TimelineMetaRowsValue*> TimelineMetaRows::getSelectedValues()
      {
      QList<TimelineMetaRowsValue*> selectedValues;
      if (!score())
            return selectedValues;

      const Selection& scoreSelection = score()->selection();

      for (Element* element : scoreSelection.elements()) {
            for (TimelineMetaRowsValue* value : _metaList) {
                  if (value->contains(element))
                        selectedValues.append(value);
                  }
            }

      return selectedValues;
      }

//---------------------------------------------------------
//   drawRows
//---------------------------------------------------------

void TimelineMetaRows::drawRows()
      {
      if (!score())
            return;

      int gridWidth = cellWidth() * score()->nmeasures();
      int nRows = getParent()->nVisibleMetaRows() + 1; // One row for measures

      for (int row = 0; row < nRows; row++) {
            QGraphicsRectItem* rect = new QGraphicsRectItem(0, cellHeight() * row, gridWidth, cellHeight());
            scene()->addItem(rect);
            }

      drawMeasureNumbers((nRows - 1) * cellHeight());

      // Use 1 to make sure rect borders are in the sceneRect
      setSceneRect(-1, -1, gridWidth + 1, cellHeight() * nRows + 1);
      }

void TimelineMetaRows::updateMetas()
      {
      _metaList.clear();
      if (!score())
            return;

      QList<int> correctRowIndexes = getParent()->getCorrectMetaRows(); // Lookup using TimelineMeta::MetaRow
      _globalZValue = 1;

      int keySigRowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::KEY_SIGNATURE)];
      if (keySigRowIndex != -1) {
            bool foundStartingKeySig = false;
            for (Segment* segment = score()->firstMeasure()->first(); segment->tick() <= 0; segment = segment->next()) {
                  if (segment->isKeySigType()) {
                        foundStartingKeySig = true;
                        break;
                        }
                  }
            if (!foundStartingKeySig) // Draws Key::C
                  drawKeySigMeta(nullptr, 0, keySigRowIndex);
            }

      int cellNumber = 0;
      for (Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
            int x = cellNumber * cellWidth();
            int rowIndex;

            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                  rowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::TEMPO)];
                  std::vector<Element*> tempoAnnotations = segment->findAnnotations(ElementType::TEMPO_TEXT, 0, score()->nstaves() * VOICES);
                  if (rowIndex != -1 && !tempoAnnotations.empty())
                        drawTempoMeta(tempoAnnotations, x, rowIndex);

                  rowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::TIME_SIGNATURE)];
                  if (rowIndex != -1 && segment->isTimeSigType())
                        drawTimeSigMeta(segment, x, rowIndex);

                  rowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::REHEARSAL_MARK)];
                  std::vector<Element*> rehearsalAnnotations = segment->findAnnotations(ElementType::REHEARSAL_MARK, 0, score()->nstaves() * VOICES);
                  if (rowIndex != -1 && !rehearsalAnnotations.empty())
                        drawRehersalMarkMeta(rehearsalAnnotations, x, rowIndex);

                  rowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::KEY_SIGNATURE)];
                  if (rowIndex != -1 && segment->isKeySigType())
                        drawKeySigMeta(segment, x, rowIndex);

                  rowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::BARLINE)];
                  if (rowIndex != -1 && (segment->isBarLine() || segment->isStartRepeatBarLineType() ||
                                         segment->isBeginBarLineType() || segment->isEndBarLineType()))
                        drawBarlineMeta(segment, x, rowIndex);
                  }

            rowIndex = correctRowIndexes[int(TimelineMeta::MetaRow::JUMPS_AND_MARKERS)];
            if (rowIndex != -1)
                  drawJumpMarkersMeta(measure->el(), x, rowIndex);
            cellNumber++;
            resetStagger();
            }
      }

//---------------------------------------------------------
//   drawMetaValue
//---------------------------------------------------------

void TimelineMetaRows::drawMetaValue(Element* element, QString text, int x, int row)
      {
      QString cleanText = text.replace("\n", "");
      TimelineMetaRowsValue* value = new TimelineMetaRowsValue(this, element, cleanText, x + getStagger(row), row * cellHeight());
      int newZ = getNewZValue();
      value->setZ(newZ);

      scene()->addItem(value);
      _metaList.append(value);
      }

//---------------------------------------------------------
//   getStagger
//---------------------------------------------------------

int TimelineMetaRows::getStagger(int x)
      {
      int staggerAmount;
      if (getParent()->collapsed())
            staggerAmount = _staggerArray[0]++;
      else
            staggerAmount = _staggerArray[x]++;
      return staggerAmount * _staggerDistance;
      }

//---------------------------------------------------------
//   resetStagger
//---------------------------------------------------------

void TimelineMetaRows::resetStagger()
      {
      for (int i = 0; i < _staggerArray.length(); i++)
            _staggerArray[i] = 0;
      }

//---------------------------------------------------------
//   drawTempoMeta
//---------------------------------------------------------

void TimelineMetaRows::drawTempoMeta(std::vector<Element*> elist, int x, int row)
      {
      for (Element* element : elist) {
            TempoText* text = toTempoText(element);
            drawMetaValue(element, text->plainText(), x, row);
            }
      }

//---------------------------------------------------------
//   drawTimeSigMeta
//---------------------------------------------------------

void TimelineMetaRows::drawTimeSigMeta(Segment* segment, int x, int row)
      {
      const TimeSig* timeSigToMatch = toTimeSig(segment->element(0));
      if (!timeSigToMatch)
            return;

      for (int track = VOICES; track < score()->nstaves(); track += VOICES) {
            const TimeSig* timeSig = toTimeSig(segment->element(track));
            if (!timeSig)
                  return;
            if (!(*timeSig == *timeSigToMatch))
                  return;
            }

      QString text = QString::number(timeSigToMatch->numerator()) +
                     QString("/") +
                     QString::number(timeSigToMatch->denominator());
      drawMetaValue(segment, text, x, row);
      }

//---------------------------------------------------------
//   drawRehersalMarkMeta
//---------------------------------------------------------

void TimelineMetaRows::drawRehersalMarkMeta(std::vector<Element*> elist, int x, int row)
      {
      for (Element* element : elist) {
            RehearsalMark* text = toRehearsalMark(element);
            drawMetaValue(element, text->plainText(), x, row);
            }
      }

//---------------------------------------------------------
//   drawKeySigMeta
//---------------------------------------------------------

void TimelineMetaRows::drawKeySigMeta(Segment* segment, int x, int row)
      {
      if (!segment) {
            drawMetaValue(segment, getKeyText(Key::C), x, row);
            return;
            }

      QMap<Key, int> keyFrequencies; // Most common key is selected
      int track = 0;
      for (Staff* staff : score()->staves()) {
            if (!staff->show() || !staff->isPitchedStaff(segment->tick())) {
                  track += VOICES;
                  continue;
                  }

            KeySig* keySig = toKeySig(segment->element(track));
            if (!keySig || keySig->generated())
                  return;

            Key key = keySig->key();
            if (keySig->isAtonal())
                  key = Key::INVALID;
            else if (keySig->isCustom())
                  key = Key::NUM_OF;
            else {
                  // TODO: Instruments not in concert pitch return different keys.
                  // Also, when in concert pitch, the key changes
                  const Interval interval = staff->part()->instrument()->transpose();
                  key = transposeKey(key, interval);
                  }

            keyFrequencies[key]++;

            track += VOICES;
            }

      Key mostFrequentKey = Key::C;
      int frequency = 0;
      for (auto keyFrequency : keyFrequencies.toStdMap()) {
            if (keyFrequency.second > frequency) {
                  mostFrequentKey = keyFrequency.first;
                  frequency = keyFrequency.second;
                  }
            }

      QString keyText = getKeyText(mostFrequentKey);
      drawMetaValue(segment, keyText, x, row);
      }

//---------------------------------------------------------
//   getKeyText
//---------------------------------------------------------

QString TimelineMetaRows::getKeyText(Key key)
      {
      if (key == Key::INVALID)
            return "X";
      else if (key == Key::NUM_OF)
            return "?";
      else if (int(key) == 0)
            return "♮";
      else if (int(key) < 0)
            return QString::number(-int(key)) + "♭";
      else
            return QString::number(int(key)) + "♯";
      }

//---------------------------------------------------------
//   drawBarlineMeta
//---------------------------------------------------------

void TimelineMetaRows::drawBarlineMeta(Segment* segment, int x, int row)
      {
      BarLine* barline = toBarLine(segment->element(0));
      if (barline) {

            QString text = "";
            switch (barline->barLineType()) {
                  case BarLineType::DOUBLE:
                        text = "𝄁";
                        break;
                  case BarLineType::END:
                        text = "𝄂";
                        break;
                  case BarLineType::END_REPEAT:
                        text = "𝄇";
                        break;
                  case BarLineType::START_REPEAT:
                        text = "𝄆";
                        break;
                  default:
                        return;
                  }

            drawMetaValue(segment, text, x, row);
            }
      }

//---------------------------------------------------------
//   drawJumpMarkersMeta
//---------------------------------------------------------

void TimelineMetaRows::drawJumpMarkersMeta(std::vector<Element*> elist, int x, int row)
      {
      QString text;
      for (Element* element : elist) {
            if (!element)
                  continue;
            else if (element->isJump()) {
                  Jump* jump = toJump(element);
                  text = jump->plainText();
                  drawMetaValue(element, text, x, row);
                  }
            else if (element->isMarker()) {
                  Marker* marker = toMarker(element);
                  text = marker->plainText();
                  drawMetaValue(element, text, x, row);
                  }
            }
      }

//---------------------------------------------------------
//   drawMeasureNumbers
//---------------------------------------------------------

void TimelineMetaRows::drawMeasureNumbers(int y)
      {
      const int increment = getMeasureIncrement();
      int measureCounter = -2; // Starting offset displays first measure number
      int cellNumber = 0;

      int gridWidth = cellWidth() * score()->nmeasures();

      for (Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
            if (measureCounter < 0) {
                  QString measureNumber = (measure->isIrregular())? "( )" : QString::number(measure->no() + 1);
                  QGraphicsTextItem* measureNumberItem = new QGraphicsTextItem(measureNumber);
                  measureNumberItem->setFont(getParent()->getParent()->getFont());

                  // TODO: Keep first measure number within grid at all times
                  qreal x = cellNumber * cellWidth();
                  QPointF targetCenter = QPointF(x + cellWidth() / 2.0, y + cellHeight() / 2.0);
                  QPointF offset = targetCenter - measureNumberItem->boundingRect().center();
                  measureNumberItem->setPos(measureNumberItem->boundingRect().translated(offset).topLeft());

                  // Don't display if extends past grid length
                  QSizeF textSize = QFontMetricsF(measureNumberItem->font()).size(Qt::TextSingleLine, measureNumber);
                  qreal rightSideOfText = measureNumberItem->boundingRect().center().x() + textSize.width() / 2.0;
                  rightSideOfText += measureNumberItem->pos().x();
                  if (rightSideOfText <= gridWidth)
                        scene()->addItem(measureNumberItem);

                  measureCounter += increment;
                  }
            measureCounter--;
            cellNumber++;
            }
      }

//---------------------------------------------------------
//   getMeasureIncrement
//---------------------------------------------------------

int TimelineMetaRows::getMeasureIncrement()
      {
      int halfway = (minCellWidth() + maxCellWidth()) / 2;
      if (cellWidth() <= maxCellWidth() && cellWidth() > halfway)
            return 1;
      else if (cellWidth() <= halfway && cellWidth() > minCellWidth())
            return 5;
      else
            return 10;
      }

//---------------------------------------------------------
//   getTopItem
//   Highest original z value is selected
//---------------------------------------------------------

TimelineMetaRowsValue* TimelineMetaRows::getTopItem(QList<QGraphicsItem*> items)
      {
      TimelineMetaRowsValue* top = nullptr;
      for (QGraphicsItem* item : items) {
            // Use the rect for mouse selection and the meta value for zValue comparison
            QGraphicsRectItem* value = qgraphicsitem_cast<QGraphicsRectItem*>(item);
            if (!value)
                  continue;

            if (value && value->parentItem()) {
                  TimelineMetaRowsValue* parent = qgraphicsitem_cast<TimelineMetaRowsValue*>(value->parentItem());
                  if (!top)
                        top = parent;
                  else if (parent->getZ() > top->getZ())
                        top = parent;
                  }
            }
      return top;
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TimelineMetaRows::mousePressEvent(QMouseEvent* event)
      {
      if (!score())
            return;

      QList<QGraphicsItem*> items = scene()->items(mapToScene(event->pos()));
      if (items.isEmpty()) {
            score()->deselectAll();
            mscore->endCmd();
            return;
            }
      TimelineMetaRowsValue* top = getTopItem(items);
      selectValue(top);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void TimelineMetaRows::mouseMoveEvent(QMouseEvent* event)
      {
      QList<QGraphicsItem*> items = scene()->items(mapToScene(event->pos()));
      if (items.isEmpty()) {
            resetOldHover();
            return;
            }
      TimelineMetaRowsValue* top = getTopItem(items);
      bringToFront(top);
      }

//---------------------------------------------------------
//   selectValue
//---------------------------------------------------------

void TimelineMetaRows::selectValue(TimelineMetaRowsValue* value)
      {
      if (!value)
            score()->deselectAll();
      else {
            value->selectValue();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   bringToFront
//---------------------------------------------------------

void TimelineMetaRows::bringToFront(TimelineMetaRowsValue* value)
      {
      if (!value)
            resetOldHover();
      else if (value != _oldHoverValue) {
            resetOldHover();
            value->setZValue(INT_MAX);
            value->hoverBrush();
            _oldHoverValue = value;
            }
      }

//---------------------------------------------------------
//   resetOldHover
//---------------------------------------------------------

void TimelineMetaRows::resetOldHover()
      {
      // TODO: Reset hover if mouse moves out of the view
      if (_oldHoverValue) {
            _oldHoverValue->resetZ();
            _oldHoverValue->resetBrush();
            _oldHoverValue = nullptr;
            }
      }

//---------------------------------------------------------
//   cellWidth
//---------------------------------------------------------

int TimelineMetaRows::cellWidth()
      {
      return getParent()->getParent()->cellWidth();
      }

//---------------------------------------------------------
//   cellHeight
//---------------------------------------------------------

int TimelineMetaRows::cellHeight()
      {
      return getParent()->getParent()->cellHeight();
      }

//---------------------------------------------------------
//   minCellWidth
//---------------------------------------------------------

int TimelineMetaRows::minCellWidth()
      {
      return getParent()->getParent()->minCellWidth();
      }

//---------------------------------------------------------
//   maxCellWidth
//---------------------------------------------------------

int TimelineMetaRows::maxCellWidth()
      {
      return getParent()->getParent()->maxCellWidth();
      }

//---------------------------------------------------------
//   getParent
//---------------------------------------------------------

TimelineMeta* TimelineMetaRows::getParent()
      {
      return static_cast<TimelineMeta*>(parent());
      }

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* TimelineMetaRows::score()
      {
      return getParent()->score();
      }

//---------------------------------------------------------
//   scoreView
//---------------------------------------------------------

ScoreView* TimelineMetaRows::scoreView()
      {
      return getParent()->scoreView();
      }

//---------------------------------------------------------
//   TimelineMeta
//---------------------------------------------------------

TimelineMeta::TimelineMeta(Timeline* parent)
   : QSplitter(parent)
      {
      // This order must match the order of TimelineMeta::MetaRow
      _metas.append(Meta(tr("Tempo"), int(MetaRow::TEMPO)));
      _metas.append(Meta(tr("Time Signature"), int(MetaRow::TIME_SIGNATURE)));
      _metas.append(Meta(tr("Rehersal Mark"), int(MetaRow::REHEARSAL_MARK)));
      _metas.append(Meta(tr("Key Signature"), int(MetaRow::KEY_SIGNATURE)));
      _metas.append(Meta(tr("Barline"), int(MetaRow::BARLINE)));
      _metas.append(Meta(tr("Jumps and Markers"), int(MetaRow::JUMPS_AND_MARKERS)));

      addWidget(new TimelineMetaLabels(this));
      addWidget(new TimelineMetaRows(this));

//      QList<int> sizes;
//      //TODO: Replace 70 with hard coded value
//      sizes << 70 << 10000;
//      setSizes(sizes);
      }

//---------------------------------------------------------
//   updateMeta
//---------------------------------------------------------

void TimelineMeta::updateMeta()
      {
      labelView()->updateLabels();
      rowsView()->updateRows();
      }

//---------------------------------------------------------
//   dataSplitterMoved
//---------------------------------------------------------

void TimelineMeta::dataSplitterMoved()
      {
      QSplitter* dataSplitter = getParent()->dataWidget();
      setSizes(dataSplitter->sizes());
      }

//---------------------------------------------------------
//   getParent
//---------------------------------------------------------

Timeline* TimelineMeta::getParent()
      {
      return static_cast<Timeline*>(parent()->parent());
      }

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* TimelineMeta::score()
      {
      return getParent()->score();
      }

//---------------------------------------------------------
//   scoreView
//---------------------------------------------------------

ScoreView* TimelineMeta::scoreView()
      {
      return getParent()->scoreView();
      }

//---------------------------------------------------------
//   getCorrectMetaRows
//   Using the current ordering, find the correct meta row indexes
//   Then, sort them into their original order
//   Hidden metas get a value of -1
//---------------------------------------------------------

QList<int> TimelineMeta::getCorrectMetaRows()
      {
      QList<int> unorderedRowIndexes;
      int counter = 0;
      for (Meta meta : _metas) {
            if (meta.visible) {
                  if (_collapsed) // Ignore counter
                        counter = 0;
                  unorderedRowIndexes.append(counter);
                  counter++;
                  }
            else
                  unorderedRowIndexes.append(-1);
            }

      QList<int> orderedRowIndexes;
      for (int i = 0; i < unorderedRowIndexes.length(); i++)
            orderedRowIndexes << 0;

      for (int i = 0; i < unorderedRowIndexes.length(); i++) {
            orderedRowIndexes[_metas[i].order] = unorderedRowIndexes[i];
            }

      return orderedRowIndexes;
      }

//---------------------------------------------------------
//   nVisibleMetaRows
//---------------------------------------------------------

int TimelineMeta::nVisibleMetaRows()
      {
      int num = 0;
      for (Meta meta : _metas) {
            if (meta.visible)
                  num++;
            }
      return num;
      }

}
