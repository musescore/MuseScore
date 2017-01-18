//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "box.h"
#include "textframe.h"
#include "text.h"
#include "score.h"
#include "barline.h"
#include "repeat.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"
#include "mscore.h"
#include "stafftext.h"
#include "icon.h"
#include "xml.h"
#include "measure.h"
#include "undo.h"

namespace Ms {


//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
   : MeasureBase(score)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Box::layout()
      {
      MeasureBase::layout();
      for (Element* e : el()) {
            if (!e->isLayoutBreak())
                  e->layout();
            }
      }

//---------------------------------------------------------
//   computeMinWidth
//---------------------------------------------------------

void HBox::computeMinWidth()
      {
      setWidth(point(boxWidth()) + topGap() + bottomGap());  // top/bottom is really left/right
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Box::draw(QPainter* painter) const
      {
      if (score() && score()->printing())
            return;
      if (selected() || editMode || dropTarget() || score()->showFrames()) {
            qreal w = spatium() * .15;
            QPainterPathStroker stroker;
            stroker.setWidth(w);
            stroker.setJoinStyle(Qt::MiterJoin);
            stroker.setCapStyle(Qt::SquareCap);

            QVector<qreal> dashes ;
            dashes.append(1);
            dashes.append(3);
            stroker.setDashPattern(dashes);
            QPainterPath path;
            w *= .5;
            path.addRect(bbox().adjusted(w, w, -w, -w));
            QPainterPath stroke = stroker.createStroke(path);
            painter->setBrush(Qt::NoBrush);
            painter->fillPath(stroke, (selected() || editMode || dropTarget()) ? MScore::selectColor[0] : MScore::frameMarginColor);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Box::startEdit(MuseScoreView*, const QPointF&)
      {
      editMode = true;
      if (isHBox())
            undoPushProperty(P_ID::BOX_WIDTH);
      else
            undoPushProperty(P_ID::BOX_HEIGHT);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(MuseScoreView*, Grip, int /*key*/, Qt::KeyboardModifiers, const QString&)
      {
      return false;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Box::editDrag(const EditData& ed)
      {
      if (isVBox()) {
            _boxHeight = Spatium((ed.pos.y() - abbox().y()) / spatium());
            if (ed.vRaster) {
                  qreal vRaster = 1.0 / MScore::vRaster();
                  int n = lrint(_boxHeight.val() / vRaster);
                  _boxHeight = Spatium(vRaster * n);
                  }
            bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
            system()->setHeight(height());
            score()->setLayout(tick());
            }
      else {
            _boxWidth += Spatium(ed.delta.x() / spatium());
            if (ed.hRaster) {
                  qreal hRaster = 1.0 / MScore::hRaster();
                  int n = lrint(_boxWidth.val() / hRaster);
                  _boxWidth = Spatium(hRaster * n);
                  }
            score()->setLayout(tick());
            }
      layout();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Box::endEdit()
      {
      editMode = false;
      layout();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Box::updateGrips(Grip* defaultGrip, QVector<QRectF>& grip) const
      {
      *defaultGrip = Grip::START;
      QRectF r(abbox());
      if (isHBox())
            grip[0].translate(QPointF(r.right(), r.top() + r.height() * .5));
      else if (type() == ElementType::VBOX)
            grip[0].translate(QPointF(r.x() + r.width() * .5, r.bottom()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Box::write(XmlWriter& xml) const
      {
      xml.stag(name());
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Box::writeProperties(XmlWriter& xml) const
      {
      writeProperty(xml, P_ID::BOX_HEIGHT);
      writeProperty(xml, P_ID::BOX_WIDTH);
      writeProperty(xml, P_ID::TOP_GAP);
      writeProperty(xml, P_ID::BOTTOM_GAP);
      writeProperty(xml, P_ID::LEFT_MARGIN);
      writeProperty(xml, P_ID::RIGHT_MARGIN);
      writeProperty(xml, P_ID::TOP_MARGIN);
      writeProperty(xml, P_ID::BOTTOM_MARGIN);

      Element::writeProperties(xml);
      for (const Element* e : el())
            e->write(xml);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(XmlReader& e)
      {
      _leftMargin      = 0.0;
      _rightMargin     = 0.0;
      _topMargin       = 0.0;
      _bottomMargin    = 0.0;
      _boxHeight       = Spatium(0);     // override default set in constructor
      _boxWidth        = Spatium(0);
      bool keepMargins = false;        // whether original margins have to be kept when reading old file

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "HBox") {
                  HBox* hb = new HBox(score());
                  hb->read(e);
                  add(hb);
                  keepMargins = true;     // in old file, box nesting used outer box margins
                  }
            else if (tag == "VBox") {
                  VBox* vb = new VBox(score());
                  vb->read(e);
                  add(vb);
                  keepMargins = true;     // in old file, box nesting used outer box margins
                  }
            else if (!Box::readProperties(e))
                  e.unknown();
            }

      // with .msc versions prior to 1.17, box margins were only used when nesting another box inside this box:
      // for backward compatibility set them to 0 in all other cases

      if (score()->mscVersion() <= 114 && (isHBox() || isVBox()) && !keepMargins)  {
            _leftMargin   = 0.0;
            _rightMargin  = 0.0;
            _topMargin    = 0.0;
            _bottomMargin = 0.0;
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Box::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "height")
            _boxHeight = Spatium(e.readDouble());
      else if (tag == "width")
            _boxWidth = Spatium(e.readDouble());
      else if (tag == "topGap") {
            _topGap = e.readDouble();
            if (score()->mscVersion() >= 206)
                  _topGap *= score()->spatium();
            topGapStyle = PropertyFlags::UNSTYLED;
            }
      else if (tag == "bottomGap") {
            _bottomGap = e.readDouble();
             if (score()->mscVersion() >= 206)
                  _bottomGap *= score()->spatium();
            bottomGapStyle = PropertyFlags::UNSTYLED;
            }
      else if (tag == "leftMargin")
            _leftMargin = e.readDouble();
      else if (tag == "rightMargin")
            _rightMargin = e.readDouble();
      else if (tag == "topMargin")
            _topMargin = e.readDouble();
      else if (tag == "bottomMargin")
            _bottomMargin = e.readDouble();
      else if (tag == "Text") {
            Text* t;
            if (isTBox()) {
                  t = toTBox(this)->text();
                  t->read(e);
                  }
            else {
                  t = new Text(score());
                  t->read(e);
                  if (t->empty()) {
                        qDebug("read empty text");
                        }
                  else
                        add(t);
                  }
            }
      else if (tag == "Symbol") {
            Symbol* s = new Symbol(score());
            s->read(e);
            add(s);
            }
      else if (tag == "Image") {
            if (MScore::noImages)
                  e.skipCurrentElement();
            else {
                  Image* image = new Image(score());
                  image->setTrack(e.track());
                  image->read(e);
                  add(image);
                  }
            }
      else if (tag == "FretDiagram") {
            FretDiagram* f = new FretDiagram(score());
            f->read(e);
            add(f);
            }
      else if (tag == "HBox") {
            HBox* hb = new HBox(score());
            hb->read(e);
            add(hb);
            }
      else if (tag == "VBox") {
            VBox* vb = new VBox(score());
            vb->read(e);
            add(vb);
            }
      else if (MeasureBase::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to Box
//---------------------------------------------------------

void Box::add(Element* e)
      {
      if (e->type() == ElementType::TEXT)
            static_cast<Text*>(e)->setLayoutToParentWidth(true);
      MeasureBase::add(e);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Box::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::BOX_HEIGHT:
                  return _boxHeight;
            case P_ID::BOX_WIDTH:
                  return _boxWidth;
            case P_ID::TOP_GAP:
                  return _topGap;
            case P_ID::BOTTOM_GAP:
                  return _bottomGap;
            case P_ID::LEFT_MARGIN:
                  return _leftMargin;
            case P_ID::RIGHT_MARGIN:
                  return _rightMargin;
            case P_ID::TOP_MARGIN:
                  return _topMargin;
            case P_ID::BOTTOM_MARGIN:
                  return _bottomMargin;
            default:
                  return MeasureBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Box::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            case P_ID::BOX_HEIGHT:
                  _boxHeight = v.value<Spatium>();
                  break;
            case P_ID::BOX_WIDTH:
                  _boxWidth = v.value<Spatium>();
                  break;
            case P_ID::TOP_GAP:
                  _topGap = v.toDouble();
                  topGapStyle = PropertyFlags::UNSTYLED;
                  break;
            case P_ID::BOTTOM_GAP:
                  _bottomGap = v.toDouble();
                  bottomGapStyle = PropertyFlags::UNSTYLED;
                  break;
            case P_ID::LEFT_MARGIN:
                  _leftMargin = v.toDouble();
                  break;
            case P_ID::RIGHT_MARGIN:
                  _rightMargin = v.toDouble();
                  break;
            case P_ID::TOP_MARGIN:
                  _topMargin = v.toDouble();
                  break;
            case P_ID::BOTTOM_MARGIN:
                  _bottomMargin = v.toDouble();
                  break;
            default:
                  return MeasureBase::setProperty(propertyId, v);
            }
      score()->setLayout(tick());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Box::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::BOX_HEIGHT:
            case P_ID::BOX_WIDTH:
                  return Spatium(0.0);

            case P_ID::TOP_GAP:
                  return isHBox() ? 0.0 : score()->styleP(StyleIdx::systemFrameDistance);
            case P_ID::BOTTOM_GAP:
                  return isHBox() ? 0.0 : score()->styleP(StyleIdx::frameSystemDistance);

            case P_ID::LEFT_MARGIN:
            case P_ID::RIGHT_MARGIN:
            case P_ID::TOP_MARGIN:
            case P_ID::BOTTOM_MARGIN:
                  return 0.0;
            default:
                  return MeasureBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags Box::propertyFlags(P_ID id) const
      {
      switch (id) {
            case P_ID::TOP_GAP:
                  return topGapStyle;
            case P_ID::BOTTOM_GAP:
                  return bottomGapStyle;
            default:
                  return MeasureBase::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Box::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::TOP_GAP:
                  setTopGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::systemFrameDistance));
                  topGapStyle = PropertyFlags::STYLED;
                  break;
            case P_ID::BOTTOM_GAP:
                  setBottomGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::frameSystemDistance));
                  bottomGapStyle = PropertyFlags::STYLED;
                  break;
            default:
                  return MeasureBase::resetProperty(id);
            }
      score()->setLayout(tick());
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Box::styleChanged()
      {
      if (topGapStyle == PropertyFlags::STYLED)
            setTopGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::systemFrameDistance));
      if (bottomGapStyle == PropertyFlags::STYLED)
            setBottomGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::frameSystemDistance));
      score()->setLayout(tick());
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Box::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::TOP_GAP:
                  return isHBox() ? StyleIdx::NOSTYLE : StyleIdx::systemFrameDistance;
            case P_ID::BOTTOM_GAP:
                  return isHBox() ? StyleIdx::NOSTYLE : StyleIdx::frameSystemDistance;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   copyValues
//---------------------------------------------------------

void Box::copyValues(Box* origin)
      {
      _boxHeight    = origin->boxHeight();
      _boxWidth     = origin->boxWidth();

      qreal factor  = magS() / origin->magS();
      _bottomGap    = origin->bottomGap() * factor;
      _topGap       = origin->topGap() * factor;
      _bottomMargin = origin->bottomMargin() * factor;
      _topMargin    = origin->topMargin() * factor;
      _leftMargin   = origin->leftMargin() * factor;
      _rightMargin  = origin->rightMargin() * factor;
      }

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(Score* score)
   : Box(score)
      {
      setBoxWidth(Spatium(5.0));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HBox::layout()
      {
      if (parent() && parent()->type() == ElementType::VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x = vb->leftMargin() * DPMM;
            qreal y = vb->topMargin() * DPMM;
            qreal w = point(boxWidth());
            qreal h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * DPMM;
            setPos(x, y);
            bbox().setRect(0.0, 0.0, w, h);
            }
      else {
            bbox().setRect(0.0, 0.0, point(boxWidth()), system()->height());
            }
      Box::layout();
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout2
//    height (bbox) is defined now
//---------------------------------------------------------

void HBox::layout2()
      {
      Box::layout();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Box::acceptDrop(const DropData& data) const
      {
      ElementType t = data.element->type();
      if (data.element->flag(ElementFlag::ON_STAFF))
            return false;
      switch (t) {
            case ElementType::LAYOUT_BREAK:
            case ElementType::TEXT:
            case ElementType::STAFF_TEXT:
            case ElementType::IMAGE:
            case ElementType::SYMBOL:
                  return true;
            case ElementType::ICON:
                  switch (toIcon(data.element)->iconType()) {
                        case IconType::VFRAME:
                        case IconType::TFRAME:
                        case IconType::FFRAME:
                        case IconType::MEASURE:
                              return true;
                        default:
                              break;
                        }
                  break;
            case ElementType::BAR_LINE:
                  return type() == ElementType::HBOX;
            default:
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Box::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->flag(ElementFlag::ON_STAFF))
            return 0;
      switch (e->type()) {
            case ElementType::LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (pageBreak() || lineBreak()) {
                        if (
                           (lb->isPageBreak() && pageBreak())
                           || (lb->isLineBreak() && lineBreak())
                           || (lb->isSectionBreak() && sectionBreak())
                           ) {
                              //
                              // if break already set
                              //
                              delete lb;
                              break;
                              }
                        for (Element* elem : el()) {
                              if (elem->type() == ElementType::LAYOUT_BREAK) {
                                    score()->undoChangeElement(elem, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // these are system elements
                  lb->setParent(this);
                  score()->undoAddElement(lb);
                  return lb;
                  }

            case ElementType::STAFF_TEXT:
                  {
                  Text* text = new Text(SubStyle::FRAME, score());
                  text->setParent(this);
                  text->setXmlText(static_cast<StaffText*>(e)->xmlText());
                  score()->undoAddElement(text);
                  delete e;
                  return text;
                  }

            case ElementType::ICON:
                  switch (toIcon(e)->iconType()) {
                        case IconType::VFRAME:
                              score()->insertMeasure(ElementType::VBOX, this);
                              break;
                        case IconType::TFRAME:
                              score()->insertMeasure(ElementType::TBOX, this);
                              break;
                        case IconType::FFRAME:
                              score()->insertMeasure(ElementType::FBOX, this);
                              break;
                        case IconType::MEASURE:
                              score()->insertMeasure(ElementType::MEASURE, this);
                              break;
                        default:
                              break;
                        }
                  break;

            case ElementType::TEXT:
            case ElementType::IMAGE:
            case ElementType::SYMBOL:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;

            case ElementType::BAR_LINE: {
                  MeasureBase* mb = next();
                  if (!mb || !mb->isMeasure()) {
                        delete e;
                        return 0;
                        }
                  score()->undoChangeBarLine(toMeasure(mb), toBarLine(e)->barLineType(), true);
                  }
                  return 0;

            default:
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF HBox::drag(EditData* data)
      {
      QRectF r(canvasBoundingRect());
      qreal diff = data->delta.x();
      qreal x1   = userOff().x() + diff;
      if (parent()->type() == ElementType::VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x2 = parent()->width() - width() - (vb->leftMargin() + vb->rightMargin()) * DPMM;
            if (x1 < 0.0)
                  x1 = 0.0;
            else if (x1 > x2)
                  x1 = x2;
            }
      setUserOff(QPointF(x1, 0.0));
      setStartDragPosition(data->delta);
      return canvasBoundingRect() | r;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void HBox::endEditDrag(const EditData&)
      {
      score()->setLayout(tick());
      score()->update();
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
      {
      return parent() && (parent()->isHBox() || parent()->isVBox());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant HBox::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::CREATE_SYSTEM_HEADER:
                  return createSystemHeader();
            default:
                  return Box::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool HBox::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::CREATE_SYSTEM_HEADER:
                  setCreateSystemHeader(v.toBool());
                  score()->setLayout(tick());
                  break;
            default:
                  return Box::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant HBox::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::CREATE_SYSTEM_HEADER:
                  return true;
            default:
                  return Box::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(Score* score)
   : Box(score)
      {
      setBoxHeight(Spatium(10.0));
      setTopGap(score->styleP(StyleIdx::systemFrameDistance));
      setBottomGap(score->styleP(StyleIdx::frameSystemDistance));
      setLineBreak(true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VBox::layout()
      {
      setPos(QPointF());      // !?
      if (system())
            bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
      else
            bbox().setRect(0.0, 0.0, 50, 50);
      Box::layout();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF VBox::getGrip(Grip) const
      {
      return QPointF(0.0, boxHeight().val());
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void VBox::setGrip(Grip, const QPointF& pt)
      {
      setBoxHeight(Spatium(pt.y()));
      layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FBox::layout()
      {
//      setPos(QPointF());      // !?
      bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
      Box::layout();
      }

//---------------------------------------------------------
//   add
///   Add new Element \a e to fret diagram box
//---------------------------------------------------------

void FBox::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == ElementType::FRET_DIAGRAM) {
//            FretDiagram* fd = static_cast<FretDiagram*>(e);
//            fd->setFlag(ElementFlag::MOVABLE, false);
            }
      else {
            qDebug("FBox::add: element not allowed");
            return;
            }
      el().push_back(e);
      }

}

