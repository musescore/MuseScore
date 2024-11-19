/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart' hide View;

import 'PositionedWidget.dart';
import 'View.dart';
import 'package:KDDockWidgetsBindings/Bindings.dart' as KDDockWidgetBindings;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;

import 'View_mixin.dart';

class Separator extends View {
  late final KDDWBindingsCore.Separator m_controller;
  late final KDDWBindingsFlutter.View m_parent;

  Separator(KDDWBindingsCore.Separator? separator,
      {required KDDWBindingsCore.View? parent})
      : super(separator, KDDockWidgetBindings.Core_ViewType.Separator, parent) {
    m_controller = separator!;
    m_color = Color.fromARGB(255, 213, 216, 218);
    debugName = "Separator";

    m_parent = KDDWBindingsFlutter.View.fromCache(parent!.thisCpp);
    m_parent.onChildAdded(this);
  }

  PositionedWidget createFlutterWidget() {
    return SeparatorWidget(this, widgetKey);
  }
}

class SeparatorWidget extends PositionedWidget {
  SeparatorWidget(kddwView, Key key) : super(kddwView, key: key);

  @override
  State<PositionedWidget> createState() {
    return SeparatorPositionedWidgetState(kddwView);
  }
}

class SeparatorPositionedWidgetState extends PositionedWidgetState {
  SeparatorPositionedWidgetState(var kddwView) : super(kddwView);

  @override
  Widget buildContents(BuildContext ctx) {
    final Separator separatorView = kddwView as Separator;

    // This simply wraps the default widget into a MouseRegion, so we can
    // react to mouse events
    final defaultContainer = super.buildContents(ctx);
    return Listener(
      onPointerDown: (event) {
        separatorView.m_controller.onMousePress();
      },
      onPointerUp: (event) {
        separatorView.m_controller.onMouseReleased();
      },
      onPointerMove: (event) {
        if (event.buttons != kPrimaryButton) return;

        final renderBox = (separatorView.m_parent as View_mixin)
            .widgetKey
            .currentContext
            ?.findRenderObject() as RenderBox;

        // The event is in coord space of the Separator. KDDW needs the position in
        // the coord space of the DropArea (m_parent) instead:

        var trans = renderBox.getTransformTo(null); // local to global
        trans.invert(); // global to local
        final localPos = event.transformed(trans).localPosition;

        separatorView.m_controller.onMouseMove(KDDockWidgetBindings.Point.ctor2(
            localPos.dx.toInt(), localPos.dy.toInt()));
      },
      child: MouseRegion(
          child: defaultContainer,
          cursor: separatorView.m_controller.isVertical()
              ? SystemMouseCursors.resizeDown
              : SystemMouseCursors.resizeLeft),
    );
  }
}
