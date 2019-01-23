#include "timelineabstract.h"
#include "preferences.h"

namespace Ms {

Score* TimelineComponent::_score = nullptr;
ScoreView* TimelineComponent::_scoreView = nullptr;
int TimelineComponent::_cellHeight = 10;
int TimelineComponent::_cellWidth = 10;

//---------------------------------------------------------
//   getFont
//---------------------------------------------------------

QFont TimelineComponent::getFont()
      {
      QString fontFamily = preferences.getString(PREF_UI_THEME_FONTFAMILY);
      int fontSize = preferences.getInt(PREF_UI_THEME_FONTSIZE);
      return QFont(fontFamily, fontSize);
      }

//---------------------------------------------------------
//   TimelineLabel
//---------------------------------------------------------

TimelineLabel::TimelineLabel(QGraphicsView* view, QString text, QFont font, int rowNumber, int height)
      : _view(view), _rowNumber(rowNumber), _text(text)
      {
      // Gen box (x and y pos?)
      _box = new QGraphicsRectItem(0, _rowNumber * height, 0, height);
      addToGroup(_box);
      _textItem = new QGraphicsTextItem(_text);
      _textItem->setFont(font);
      centerTextVertically();
      addToGroup(_textItem);
      }

//---------------------------------------------------------
//   updateWidth
//---------------------------------------------------------

void TimelineLabel::updateWidth(int newWidth, int handleWidth)
      {
      QRectF newRect = _box->rect();
      newRect.setWidth(newWidth);
      _box->setRect(newRect);

      QFontMetrics fontMetrics = QFontMetrics(getFont());
      QString newText = fontMetrics.elidedText(_text, Qt::ElideRight, newWidth - handleWidth);
      newText.replace("...", "…");
      _textItem->setPlainText(newText);
      }

//---------------------------------------------------------
//   centerTextVertically
//---------------------------------------------------------

void TimelineLabel::centerTextVertically()
      {
      QPointF targetCenter = _box->boundingRect().center();
      QPointF currentCenter = _textItem->boundingRect().center();
      QPointF offset = targetCenter - currentCenter;
      _textItem->setPos(0, _textItem->boundingRect().translated(offset).y());
      }

//---------------------------------------------------------
//   getTextWidth
//---------------------------------------------------------

int TimelineLabel::getTextWidth()
      {
      return _textItem->boundingRect().width();
      }

}
