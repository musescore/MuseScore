//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "effectgui.h"
#include "effect.h"
#include <QQmlContext>

namespace Ms {

//---------------------------------------------------------
//   EffectGui
//---------------------------------------------------------

EffectGui::EffectGui(Effect* e)
   : QQuickView(0)
      {
      _effect = e;
      setResizeMode(QQuickView::SizeViewToRootObject);
//      setFocusPolicy(Qt::StrongFocus);
      }

//---------------------------------------------------------
//   EffectGui
//---------------------------------------------------------

EffectGui::~EffectGui()
      {
      printf("destructor EffectGui %p\n", this);
      }


//---------------------------------------------------------
//   init
//---------------------------------------------------------

void EffectGui::init(QUrl& url)
      {
      if (_effect) {
            rootContext()->setContextProperty("myEffect", _effect);
            setSource(url);

            if (rootObject()) {
                  connect(rootObject(), SIGNAL(valueChanged(QString, qreal)),
                     SLOT(valueChanged(QString, qreal)));
                  }
            else
                  qDebug("no root object for %s", qPrintable(_effect->name()));
            }
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EffectGui::valueChanged(const QString& msg, qreal val)
      {
      if (_effect->value(msg) != val) {
            _effect->setValue(msg, val);
            emit valueChanged();
            }
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void EffectGui::updateValues()
      {
      if (rootObject()) {
            if (!QMetaObject::invokeMethod(rootObject(), "updateValues",
               Qt::DirectConnection)) {
                  qDebug("EffectGui::updateValues: failed");
                  }
            }
      }
}

