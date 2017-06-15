//////////////////////////////////////////////////////////////////////////////
// oxygenframeshadow.h
// handle sunken frames' shadows
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Largely inspired from skulpture widget style
// Copyright (c) 2007-2009 Christoph Feck <christoph@maxiom.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "frameshadow.h"
#include "colorutils.h"

//---------------------------------------------------------
//   registerWidget
//---------------------------------------------------------

bool FrameShadowFactory::registerWidget( QWidget* widget, StyleHelper& helper ) {
      if (!widget)
            return false;
      if (isRegistered(widget))
            return false;

      // check whether widget is a frame, and has the proper shape
      bool accepted = false;
      bool flat = false;

      // cast to frame and check
      QFrame* frame(qobject_cast<QFrame*>(widget));
      if (!frame)
            return false;

      // also do not install on QSplitter
      /*
       due to Qt, splitters are set with a frame style that matches the condition below,
       though no shadow should be installed, obviously
       */
      if (qobject_cast<QSplitter*>(widget))
            return false;

      // further checks on frame shape, and parent
      if ( frame->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken) )
            accepted = true;
      else if (widget->parent() && widget->parent()->inherits("QComboBoxPrivateContainer")) {
            accepted = true;
            flat = true;
            }

      if (!accepted)
            return false;

      // store in set
      _registeredWidgets.insert( widget );

      // catch object destruction
      connect(widget, SIGNAL(destroyed(QObject*)), SLOT(widgetDestroyed(QObject*)));

      // install shadow
      installShadows(widget, helper, flat);

      return true;
      }

//---------------------------------------------------------
//   unregisterWidget
//---------------------------------------------------------

void FrameShadowFactory::unregisterWidget( QWidget* widget ) {
      if (!isRegistered(widget))
            return;
      _registeredWidgets.remove(widget);
      removeShadows(widget);
      }

//---------------------------------------------------------
//   widgetDestroyed
//---------------------------------------------------------

void FrameShadowFactory::widgetDestroyed( QObject* o ) {
      _registeredWidgets.remove( o );
      }

//---------------------------------------------------------
//   installShadows
//---------------------------------------------------------

void FrameShadowFactory::installShadows(QWidget* widget, StyleHelper& helper, bool flat) {
      removeShadows(widget);

      widget->installEventFilter(this);
      if (!flat) {
            installShadow( widget, helper, Left );
            installShadow( widget, helper, Right );
            }

      installShadow( widget, helper, Top, flat );
      installShadow( widget, helper, Bottom, flat );
      }

//---------------------------------------------------------
//   removeShadows
//---------------------------------------------------------

void FrameShadowFactory::removeShadows( QWidget* widget ) {
      widget->removeEventFilter(this);

      const QList<QObject* > children = widget->children();
      foreach(QObject * child, children) {
            if (FrameShadowBase* shadow = qobject_cast<FrameShadowBase*>(child)) {
                  shadow->hide();
                  shadow->setParent(0);
                  shadow->deleteLater();
                  }
            }
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool FrameShadowFactory::eventFilter( QObject* object, QEvent* event ) {
      switch ( event->type() ) {
                  // TODO: possibly implement ZOrderChange event, to make sure that
                  // the shadow is always painted on top
            case QEvent::ZOrderChange: {
                  raiseShadows( object );
                  break;
                  }
            case QEvent::Show:
                  updateShadowsGeometry( object );
                  update( object );
                  break;
            case QEvent::Resize:
                  updateShadowsGeometry( object );
                  break;
            default:
                  break;
            }
      return QObject::eventFilter( object, event );
      }

//---------------------------------------------------------
//   updateShadowsGeometry
//---------------------------------------------------------

void FrameShadowFactory::updateShadowsGeometry(QObject* object) const {
      const QList<QObject*> children = object->children();
      foreach(QObject * child, children) {
            if (FrameShadowBase* shadow = qobject_cast<FrameShadowBase*>(child)) {
                  shadow->updateGeometry();
                  }
            }
      }

//---------------------------------------------------------
//   raiseShadows
//---------------------------------------------------------

void FrameShadowFactory::raiseShadows(QObject* object) const {
      const QList<QObject*> children = object->children();
      foreach(QObject * child, children) {
            if (FrameShadowBase* shadow = qobject_cast<FrameShadowBase*>(child)) {
                  shadow->raise();
                  }
            }
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void FrameShadowFactory::update( QObject* object ) const {
      const QList<QObject* > children = object->children();
      foreach( QObject * child, children ) {
            if (FrameShadowBase* shadow = qobject_cast<FrameShadowBase*>(child)) {
                  shadow->update();
                  }
            }
      }

//---------------------------------------------------------
//   updateState
//---------------------------------------------------------

void FrameShadowFactory::updateState(const QWidget* widget, bool focus, bool hover, qreal opacity, AnimationMode mode) const {
      const QList<QObject*> children = widget->children();
      foreach (QObject * child, children) {
            if (FrameShadowBase* shadow = qobject_cast<FrameShadowBase*>(child)) {
                  shadow->updateState( focus, hover, opacity, mode );
                  }
            }
      }

//---------------------------------------------------------
//   installShadow
//---------------------------------------------------------

void FrameShadowFactory::installShadow( QWidget* widget, StyleHelper& helper, ShadowArea area, bool flat ) const {
      FrameShadowBase* shadow(0);
      if (flat)
            shadow = new FlatFrameShadow(area, helper);
      else
            shadow = new SunkenFrameShadow(area, helper);
      shadow->setParent(widget);
      shadow->updateGeometry();
      shadow->show();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FrameShadowBase::init() {
      setAttribute(Qt::WA_OpaquePaintEvent, false);

      setFocusPolicy(Qt::NoFocus);
      setAttribute(Qt::WA_TransparentForMouseEvents, true);
      setContextMenuPolicy(Qt::NoContextMenu);

      // grab viewport widget
      QWidget* viewport( FrameShadowBase::viewport() );
      // set cursor from viewport
      if (viewport)
            setCursor(viewport->cursor());
      }

//---------------------------------------------------------
//   viewport
//---------------------------------------------------------

QWidget* FrameShadowBase::viewport() const {
      if (!parentWidget())
            return 0;

      if (QAbstractScrollArea* widget = qobject_cast<QAbstractScrollArea*>(parentWidget()))
            return widget->viewport();
      else
            return 0;
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool FrameShadowBase::event(QEvent* e) {
      // paintEvents are handled separately
      if (e->type() == QEvent::Paint)
            return QWidget::event(e);

      QWidget* viewport(FrameShadowBase::viewport());

      switch (e->type()) {
            case QEvent::DragEnter:
            case QEvent::DragMove:
            case QEvent::DragLeave:
            case QEvent::Drop:
                  if ( viewport ) {
                        setAcceptDrops(viewport->acceptDrops());
                        return viewport->QObject::event(e);
                        }
                  break;

            case QEvent::Enter:
                  if ( viewport ) {
                        setCursor(viewport->cursor());
                        setAcceptDrops(viewport->acceptDrops());
                        }
                  break;

            case QEvent::ContextMenu:
                  if ( viewport ) {
                        QContextMenuEvent* me = static_cast<QContextMenuEvent*>(e);
                        QContextMenuEvent* ne = new QContextMenuEvent(me->reason(), parentWidget()->mapFromGlobal(me->globalPos()), me->globalPos());
                        QApplication::sendEvent(viewport, ne);
                        e->accept();
                        return true;
                        }
                  break;

            case QEvent::MouseButtonPress:
                  releaseMouse();
                  // fall through
            case QEvent::MouseMove:
                  // fall through
            case QEvent::MouseButtonRelease:
                  if ( viewport ) {
                        QMouseEvent* me = static_cast<QMouseEvent*>(e);
                        QMouseEvent* ne = new QMouseEvent(e->type(), parentWidget()->mapFromGlobal(me->globalPos()), me->globalPos(), me->button(), me->buttons(), me->modifiers());
                        QApplication::sendEvent(viewport, ne);
                        e->accept();
                        return true;
                        }
                  break;

            default:
                  break;
            }
      e->ignore();
      return false;
      }

//---------------------------------------------------------
//   updateGeometry
//---------------------------------------------------------

void SunkenFrameShadow::updateGeometry() {
      QWidget* widget = parentWidget();
      if (!widget)
            return;

      QRect cr = widget->contentsRect();
      switch (shadowArea()) {
            case Top:
                  cr.setHeight( SHADOW_SIZE_TOP );
                  cr.adjust( -1, -1, 1, 0 );
                  break;
            case Left:
                  cr.setWidth(SHADOW_SIZE_LEFT);
                  cr.adjust(-1, SHADOW_SIZE_TOP, 0, -SHADOW_SIZE_BOTTOM);
                  break;
            case Bottom:
                  cr.setTop(cr.bottom() - SHADOW_SIZE_BOTTOM + 1);
                  cr.adjust( -1, 0, 1, 1 );
                  break;
            case Right:
                  cr.setLeft(cr.right() - SHADOW_SIZE_RIGHT + 1);
                  cr.adjust(0, SHADOW_SIZE_TOP, 1, -SHADOW_SIZE_BOTTOM);
                  break;
            case UnknownArea:
            default:
                  return;
            }
      setGeometry(cr);
      }

//---------------------------------------------------------
//   updateState
//---------------------------------------------------------

void SunkenFrameShadow::updateState( bool focus, bool hover, qreal opacity, AnimationMode mode ) {
      bool changed = false;
      if (_focus != focus) {
            _focus = focus;
            changed |= true;
            }
      if (_hover != hover) {
            _hover = hover;
            changed |= !_focus;
            }
      if (_mode != mode) {
            _mode = mode;
            changed |=
                  (_mode == AnimationNone) ||
                  (_mode == AnimationFocus) ||
                  (_mode == AnimationHover && !_focus );
            }

      if (_opacity != opacity) {
            _opacity = opacity;
            changed |= (_mode != AnimationNone );
            }
      if (changed) {
            if (QWidget* viewport = this->viewport()) {
                  // need to disable viewport updates to avoid some redundant painting
                  // besides it fixes one visual glitch (from Qt) in QTableViews
                  viewport->setUpdatesEnabled( false );
                  update() ;
                  viewport->setUpdatesEnabled( true );
                  }
            else
                  update();
            }
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void SunkenFrameShadow::paintEvent(QPaintEvent* event ) {
      // this fixes shadows in frames that change frameStyle() after polish()
      if (QFrame* frame = qobject_cast<QFrame*>(parentWidget())) {
            if (frame->frameStyle() != (QFrame::StyledPanel | QFrame::Sunken))
                  return;
            }

      QWidget* parent = parentWidget();
      QRect r = parent->contentsRect();
      r.translate(mapFromParent(QPoint(0, 0)));

      QColor base( palette().color(QPalette::Window) );
      TileSet::Tiles tiles;
      switch ( shadowArea() ) {
            case Top: {
                  tiles = TileSet::Left | TileSet::Top | TileSet::Right;
                  r.adjust( -2, -2, 2, -1 );
                  break;
                  }

            case Bottom: {
                  tiles = TileSet::Left | TileSet::Bottom | TileSet::Right;
                  r.adjust( -2, 1, 2, 2 );
                  break;
                  }

            case Left: {
                  tiles = TileSet::Left;
                  r.adjust( -2, -3, -1, 3 );
                  break;
                  }

            case Right: {
                  tiles = TileSet::Right;
                  r.adjust( -1, -3, 2, 3 );
                  break;
                  }

            default:
                  return;
            }

      QPainter painter(this);
      painter.setClipRegion( event->region() );
      _helper.renderHole( &painter, palette().color( QPalette::Window ), r, _focus, _hover, _opacity, _mode, tiles, true );
      }

//---------------------------------------------------------
//   updateGeometry
//---------------------------------------------------------

void FlatFrameShadow::updateGeometry() {
      QWidget* widget = parentWidget();
      if (!widget)
            return;

      QRect cr = widget->contentsRect();
      switch (shadowArea()) {
            case Top:
                  cr.setHeight( SHADOW_SIZE_TOP - 3 );
                  break;

            case Bottom:
                  cr.setTop(cr.bottom() - SHADOW_SIZE_BOTTOM + 4);
                  break;

            case UnknownArea:
            default:
                  return;
            }
      setGeometry(cr);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void FlatFrameShadow::paintEvent(QPaintEvent* event ) {
      // this fixes shadows in frames that change frameStyle() after polish()
      if (QFrame* frame = qobject_cast<QFrame*>(parentWidget())) {
            if (frame->frameStyle() != (QFrame::NoFrame))
                  return;
            }
      QWidget* parent = parentWidget();
      QImage pm(size(),  QImage::Format_ARGB32_Premultiplied);

            {
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            p.setClipRegion(event->region());
            p.setRenderHints(QPainter::Antialiasing);
            p.translate(-geometry().topLeft());
            p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            p.setPen(Qt::NoPen);
            _helper.renderMenuBackground(&p, geometry(), parent, parent->palette());

            // mask
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush(Qt::black);
            p.drawRoundedRect(QRectF(parent->contentsRect()), 2.5, 2.5);
            }

      QPainter p(this);
      p.setClipRegion(event->region());
      p.fillRect(rect(), Qt::transparent);
      p.drawPixmap(QPoint(0, 0), QPixmap::fromImage(pm));
      return;
      }

