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
//TODO-ws            score()->doLayoutPages();
            }
      else {
            _boxWidth += Spatium(ed.delta.x() / spatium());
            if (ed.hRaster) {
                  qreal hRaster = 1.0 / MScore::hRaster();
                  int n = lrint(_boxWidth.val() / hRaster);
                  _boxWidth = Spatium(hRaster * n);
                  }
            score()->setLayoutAll();
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
      else if (type() == Element::Type::VBOX)
            grip[0].translate(QPointF(r.x() + r.width() * .5, r.bottom()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Box::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Box::writeProperties(Xml& xml) const
      {
      writeProperty(xml, P_ID::BOX_HEIGHT);
      writeProperty(xml, P_ID::BOX_WIDTH);

      if (getProperty(P_ID::TOP_GAP) != propertyDefault(P_ID::TOP_GAP))
            xml.tag("topGap", _topGap / spatium());
      if (getProperty(P_ID::BOTTOM_GAP) != propertyDefault(P_ID::BOTTOM_GAP))
            xml.tag("bottomGap", _bottomGap / spatium());
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
            topGapStyle = PropertyStyle::UNSTYLED;
            }
      else if (tag == "bottomGap") {
            _bottomGap = e.readDouble();
             if (score()->mscVersion() >= 206)
                  _bottomGap *= score()->spatium();
            bottomGapStyle = PropertyStyle::UNSTYLED;
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
      if (e->type() == Element::Type::TEXT)
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
                  topGapStyle = PropertyStyle::UNSTYLED;
                  break;
            case P_ID::BOTTOM_GAP:
                  _bottomGap = v.toDouble();
                  bottomGapStyle = PropertyStyle::UNSTYLED;
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
      score()->setLayoutAll();
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
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Box::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::TOP_GAP:
                  return topGapStyle;
            case P_ID::BOTTOM_GAP:
                  return bottomGapStyle;
            default:
                  return MeasureBase::propertyStyle(id);
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
                  topGapStyle = PropertyStyle::STYLED;
                  break;
            case P_ID::BOTTOM_GAP:
                  setBottomGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::frameSystemDistance));
                  bottomGapStyle = PropertyStyle::STYLED;
                  break;
            default:
                  return MeasureBase::resetProperty(id);
            }
      score()->setLayoutAll();
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Box::styleChanged()
      {
      if (topGapStyle == PropertyStyle::STYLED)
            setTopGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::systemFrameDistance));
      if (bottomGapStyle == PropertyStyle::STYLED)
            setBottomGap(isHBox() ? 0.0 : score()->styleP(StyleIdx::frameSystemDistance));
      score()->setLayoutAll();
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
      if (parent() && parent()->type() == Element::Type::VBOX) {
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
      Element::Type type = data.element->type();
      if (data.element->flag(ElementFlag::ON_STAFF))
            return false;
      switch (type) {
            case Element::Type::LAYOUT_BREAK:
            case Element::Type::TEXT:
            case Element::Type::STAFF_TEXT:
            case Element::Type::IMAGE:
            case Element::Type::SYMBOL:
                  return true;
            case Element::Type::ICON:
                  switch(static_cast<Icon*>(data.element)->iconType()) {
                        case IconType::VFRAME:
                        case IconType::TFRAME:
                        case IconType::FFRAME:
                        case IconType::MEASURE:
                              return true;
                        default:
                              break;
                        }
                  break;
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
            case Element::Type::LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (pageBreak() || lineBreak()) {
                        if (
                           (lb->layoutBreakType() == LayoutBreak::Type::PAGE && pageBreak())
                           || (lb->layoutBreakType() == LayoutBreak::Type::LINE && lineBreak())
                           || (lb->layoutBreakType() == LayoutBreak::Type::SECTION && sectionBreak())
                           ) {
                              //
                              // if break already set
                              //
                              delete lb;
                              break;
                              }
                        for (Element* elem : el()) {
                              if (elem->type() == Element::Type::LAYOUT_BREAK) {
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

            case Element::Type::STAFF_TEXT:
                  {
                  Text* text = new Text(score());
                  text->setTextStyleType(TextStyleType::FRAME);
                  text->setParent(this);
                  text->setXmlText(static_cast<StaffText*>(e)->xmlText());
                  score()->undoAddElement(text);
                  delete e;
                  return text;
                  }

            case Element::Type::ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case IconType::VFRAME:
                              score()->insertMeasure(Element::Type::VBOX, this);
                              break;
                        case IconType::TFRAME:
                              score()->insertMeasure(Element::Type::TBOX, this);
                              break;
                        case IconType::FFRAME:
                              score()->insertMeasure(Element::Type::FBOX, this);
                              break;
                        case IconType::MEASURE:
                              score()->insertMeasure(Element::Type::MEASURE, this);
                              break;
                        default:
                              break;
                        }
                  break;

            case Element::Type::TEXT:
            case Element::Type::IMAGE:
            case Element::Type::SYMBOL:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;

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
      if (parent()->type() == Element::Type::VBOX) {
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

void HBox::endEditDrag()
      {
      score()->setLayoutAll();
      score()->update();
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
      {
      return parent() && (parent()->type() == Element::Type::HBOX || parent()->type() == Element::Type::VBOX);
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
      if (e->type() == Element::Type::FRET_DIAGRAM) {
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

