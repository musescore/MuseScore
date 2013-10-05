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

namespace Ms {

static const qreal BOX_MARGIN = 0.0;

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
   : MeasureBase(score)
      {
      editMode      = false;
      _boxWidth     = Spatium(0);
      _boxHeight    = Spatium(0);
      _leftMargin   = BOX_MARGIN;
      _rightMargin  = BOX_MARGIN;
      _topMargin    = BOX_MARGIN;
      _bottomMargin = BOX_MARGIN;
      _topGap       = 0.0;
      _bottomGap    = 0.0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Box::layout()
      {
      MeasureBase::layout();
      foreach (Element* el, _el) {
            if (el->type() != LAYOUT_BREAK)
                  el->layout();
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
      if (type() == HBOX)
            undoPushProperty(P_BOX_WIDTH);
      else
            undoPushProperty(P_BOX_HEIGHT);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(MuseScoreView*, int /*grip*/, int /*key*/, Qt::KeyboardModifiers, const QString&)
      {
      return false;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Box::editDrag(const EditData& ed)
      {
      if (type() == VBOX) {
            _boxHeight = Spatium((ed.pos.y() - abbox().y()) / spatium());
            if (ed.vRaster) {
                  qreal vRaster = 1.0 / MScore::vRaster();
                  int n = lrint(_boxHeight.val() / vRaster);
                  _boxHeight = Spatium(vRaster * n);
                  }
            bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
            system()->setHeight(height());
            score()->doLayoutPages();
            }
      else {
            _boxWidth += Spatium(ed.delta.x() / spatium());
            if (ed.hRaster) {
                  qreal hRaster = 1.0 / MScore::hRaster();
                  int n = lrint(_boxWidth.val() / hRaster);
                  _boxWidth = Spatium(hRaster * n);
                  }

            foreach(Element* e, _el) {
                  if (e->type() == TEXT) {
                        static_cast<Text*>(e)->setModified(true);  // force relayout
                        }
                  }
            score()->setLayoutAll(true);
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

void Box::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 1;
      QRectF r(abbox());
      if (type() == HBOX)
            grip[0].translate(QPointF(r.right(), r.top() + r.height() * .5));
      else if (type() == VBOX)
            grip[0].translate(QPointF(r.x() + r.width() * .5, r.bottom()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Box::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperty(xml, P_BOX_HEIGHT);
      writeProperty(xml, P_BOX_WIDTH);
      writeProperty(xml, P_TOP_GAP);
      writeProperty(xml, P_BOTTOM_GAP);
      writeProperty(xml, P_LEFT_MARGIN);
      writeProperty(xml, P_RIGHT_MARGIN);
      writeProperty(xml, P_TOP_MARGIN);
      writeProperty(xml, P_BOTTOM_MARGIN);

      Element::writeProperties(xml);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(XmlReader& e)
      {
      _leftMargin = _rightMargin = _topMargin = _bottomMargin = 0.0;
      bool keepMargins = false;        // whether original margins have to be kept when reading old file

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "height")
                  _boxHeight = Spatium(e.readDouble());
            else if (tag == "width")
                  _boxWidth = Spatium(e.readDouble());
            else if (tag == "topGap")
                  _topGap = e.readDouble();
            else if (tag == "bottomGap")
                  _bottomGap = e.readDouble();
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
                  if (type() == TBOX) {
                        t = static_cast<TBox*>(this)->getText();
                        t->read(e);
                        }
                  else {
                        t = new Text(score());
                        t->read(e);
                        add(t);
                        if (score()->mscVersion() <= 114)
                              t->setLayoutToParentWidth(true);
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  Image* image = new Image(score());
                  image->setTrack(e.track());
                  image->read(e);
                  add(image);
                  }
            else if (tag == "FretDiagram") {
                  FretDiagram* f = new FretDiagram(score());
                  f->read(e);
                  add(f);
                  }
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "HBox") {
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
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }

      // with .msc versions prior to 1.17, box margins were only used when nesting another box inside this box:
      // for backward compatibility set them to 0 in all other cases

      if (score()->mscVersion() < 117 && (type() == HBOX || type() == VBOX) && !keepMargins)  {
            _leftMargin = _rightMargin = _topMargin = _bottomMargin = 0.0;
            }
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to Box
//---------------------------------------------------------

void Box::add(Element* e)
      {
      if (e->type() == TEXT)
            static_cast<Text*>(e)->setLayoutToParentWidth(true);
      MeasureBase::add(e);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Box::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_BOX_HEIGHT:
                  return _boxHeight.val();
            case P_BOX_WIDTH:
                  return _boxWidth.val();
            case P_TOP_GAP:
                  return _topGap;
            case P_BOTTOM_GAP:
                  return _bottomGap;
            case P_LEFT_MARGIN:
                  return _leftMargin;
            case P_RIGHT_MARGIN:
                  return _rightMargin;
            case P_TOP_MARGIN:
                  return _topMargin;
            case P_BOTTOM_MARGIN:
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
            case P_BOX_HEIGHT:
                  _boxHeight = Spatium(v.toDouble());
                  break;
            case P_BOX_WIDTH:
                  _boxWidth = Spatium(v.toDouble());
                  break;
            case P_TOP_GAP:
                  _topGap = v.toDouble();
                  break;
            case P_BOTTOM_GAP:
                  _bottomGap = v.toDouble();
                  break;
            case P_LEFT_MARGIN:
                  _leftMargin = v.toDouble();
                  break;
            case P_RIGHT_MARGIN:
                  _rightMargin = v.toDouble();
                  break;
            case P_TOP_MARGIN:
                  _topMargin = v.toDouble();
                  break;
            case P_BOTTOM_MARGIN:
                  _bottomMargin = v.toDouble();
                  break;
            default:
                  return MeasureBase::setProperty(propertyId, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Box::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_BOX_HEIGHT:
            case P_BOX_WIDTH:
            case P_TOP_GAP:
            case P_BOTTOM_GAP:
                  return 0.0;

            case P_LEFT_MARGIN:
            case P_RIGHT_MARGIN:
            case P_TOP_MARGIN:
            case P_BOTTOM_MARGIN:
                  return BOX_MARGIN;
            default:
                  return MeasureBase::propertyDefault(id);
            }
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
      if (parent() && parent()->type() == VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x = vb->leftMargin() * MScore::DPMM;
            qreal y = vb->topMargin() * MScore::DPMM;
            qreal w = point(boxWidth());
            qreal h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * MScore::DPMM;
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

bool Box::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      int type = e->type();
      if(e->flag(ELEMENT_ON_STAFF))
            return false;
      switch(type) {
            case LAYOUT_BREAK:
            case TEXT:
            case STAFF_TEXT:
            case IMAGE:
            case SYMBOL:
                  return true;
            case ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case ICON_VFRAME:
                        case ICON_TFRAME:
                        case ICON_FFRAME:
                        case ICON_MEASURE:
                              return true;
                        }
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
      if(e->flag(ELEMENT_ON_STAFF))
            return 0;
      switch(e->type()) {
            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (_pageBreak || _lineBreak) {
                        if (
                           (lb->layoutBreakType() == LayoutBreak::PAGE && _pageBreak)
                           || (lb->layoutBreakType() == LayoutBreak::LINE && _lineBreak)
                           || (lb->layoutBreakType() == LayoutBreak::SECTION && _sectionBreak)
                           ) {
                              //
                              // if break already set
                              //
                              delete lb;
                              break;
                              }
                        foreach(Element* elem, _el) {
                              if (elem->type() == LAYOUT_BREAK) {
                                    score()->undoChangeElement(elem, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // this are system elements
                  lb->setParent(this);
                  score()->undoAddElement(lb);
                  return lb;
                  }
            case STAFF_TEXT:
                  {
                  Text* text = new Text(score());
                  text->setTextStyleType(TEXT_STYLE_FRAME);
                  text->setParent(this);
                  text->setText(static_cast<StaffText*>(e)->text());
                  score()->undoAddElement(text);
                  delete e;
                  return text;
                  }

            case ICON:
                  switch(static_cast<Icon*>(e)->iconType()) {
                        case ICON_VFRAME:
                              score()->insertMeasure(VBOX, this);
                              break;
                        case ICON_TFRAME:
                              score()->insertMeasure(TBOX, this);
                              break;
                        case ICON_FFRAME:
                              score()->insertMeasure(FBOX, this);
                              break;
                        case ICON_MEASURE:
                              score()->insertMeasure(MEASURE, this);
                              break;
                        }
                  break;

            default:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;
            }
      return 0;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF HBox::drag(const EditData& data)
      {
      QRectF r(canvasBoundingRect());
      qreal diff = data.delta.x();
      qreal x1   = userOff().x() + diff;
      if (parent()->type() == VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x2 = parent()->width() - width() - (vb->leftMargin() + vb->rightMargin()) * MScore::DPMM;
            if (x1 < 0.0)
                  x1 = 0.0;
            else if (x1 > x2)
                  x1 = x2;
            }
      setUserOff(QPointF(x1, 0.0));
      setStartDragPosition(data.pos);
      return canvasBoundingRect() | r;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void HBox::endEditDrag()
      {
      score()->setLayoutAll(true);
      score()->update();
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
      {
      return parent() && (parent()->type() == HBOX || parent()->type() == VBOX);
      }

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(Score* score)
   : Box(score)
      {
      setBoxHeight(Spatium(10.0));
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

QPointF VBox::getGrip(int) const
      {
      return QPointF(0.0, boxHeight().val());
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void VBox::setGrip(int, const QPointF& pt)
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
      if (e->type() == FRET_DIAGRAM) {
//            FretDiagram* fd = static_cast<FretDiagram*>(e);
//            fd->setFlag(ELEMENT_MOVABLE, false);
            }
      else {
            qDebug("FBox::add: element not allowed\n");
            return;
            }
      _el.push_back(e);
      }

}

