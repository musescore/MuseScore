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
   : QQuickWidget()
      {
      _effect = e;
      setResizeMode(QQuickWidget::SizeViewToRootObject);
//      setFocusPolicy(Qt::StrongFocus);
      connect(this, SIGNAL(statusChanged(QQuickWidget::Status)), SLOT(reportErrors(QQuickWidget::Status)));
      }

//---------------------------------------------------------
//   reportErrors
//---------------------------------------------------------

void EffectGui::reportErrors(QQuickWidget::Status status)
      {
//      printf("EffectGui::statusChange: %d\n", int(status));
      if (status == QQuickWidget::Error) {
            for (auto e : errors())
                  qDebug("EffectGui: Error: %s", qPrintable(e.toString()));
            }
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
            if (!QMetaObject::invokeMethod(rootObject(), "updateValues")) {
                  qDebug("EffectGui::updateValues: failed");
                  }
            }
      }
}

