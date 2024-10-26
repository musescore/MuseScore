/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:flutter/material.dart' hide View;
import 'package:flutter/widgets.dart';
import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;

import 'GlobalStringKey.dart';
import 'WindowOverlayWidget.dart';

/// @brief A Widget that hosts a single KDDW FloatingWindow or MainWindow
/// Since Flutter doesn't support real OS level multi-windows yet we need
/// to draw the windows ourselves
class WindowWidget extends StatefulWidget {
  late final View_mixin kddwView;

  /// Private CTOR. Use WindowWidget.fromView() instead
  WindowWidget._(KDDWBindingsCore.View view, Key key) : super(key: key) {
    kddwView = KDDWBindingsFlutter.View.fromCache(view.thisCpp) as View_mixin;
  }

  static WindowWidget fromView(KDDWBindingsCore.View viewCpp) {
    View_mixin view = View_mixin.fromCpp(viewCpp);
    final key = GlobalStringKey("KDDockWidgets_WindowWidget-${view.m_id}");
    if (key.currentWidget == null) {
      return WindowWidget._(viewCpp, key);
    } else {
      return key.currentWidget! as WindowWidget;
    }
  }

  @override
  State<StatefulWidget> createState() {
    return WindowWidgetState(kddwView);
  }
}

class WindowWidgetState extends State<WindowWidget> {
  late final View_mixin kddwView;
  WindowWidgetState(this.kddwView);

  void onGeometryChanged() {
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    final geo = kddwView.viewGeometry();
    final x = geo.x().toDouble();
    final y = geo.y().toDouble();
    final width = geo.width();
    final height = geo.height();

    final parentRB = WindowOverlayWidget.globalKey()
        .currentContext!
        .findRenderObject() as RenderBox?;

    final localPos =
        parentRB == null ? Offset(x, y) : parentRB.globalToLocal(Offset(x, y));

    return Positioned(
        left: localPos.dx,
        top: localPos.dy,
        width: width.toDouble(),
        height: height.toDouble(),
        child: kddwView.flutterWidget);
  }
}
