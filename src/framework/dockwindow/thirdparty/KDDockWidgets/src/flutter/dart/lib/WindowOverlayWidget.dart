/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/WindowWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:flutter/material.dart' hide View;
import 'package:flutter/widgets.dart';
import 'package:KDDockWidgets/Platform.dart';

import 'GlobalStringKey.dart';

/// @brief A Widget to host KDDW floating windows
/// Since Flutter doesn't support real OS level multi-windows we need
/// to draw the floating windows into an overlay
class WindowOverlayWidget extends StatefulWidget {
  // We only have one overlay per application
  static GlobalStringKey<WindowOverlayWidgetState> globalKey() {
    return GlobalStringKey<WindowOverlayWidgetState>("windowOverlayState");
  }

  late final bool supportsMainWindow;
  WindowOverlayWidget({bool supportsMainWindow = false})
      : super(key: globalKey()) {
    this.supportsMainWindow = supportsMainWindow;
  }

  @override
  State<StatefulWidget> createState() {
    return WindowOverlayWidgetState(supportsMainWindow: supportsMainWindow);
  }
}

class WindowOverlayWidgetState extends State<WindowOverlayWidget> {
  late final bool supportsMainWindow;
  WindowOverlayWidgetState({required bool supportsMainWindow}) {
    this.supportsMainWindow = supportsMainWindow;
  }

  @override
  Widget build(BuildContext context) {
    List<Widget> windowWidgets = [];

    if (supportsMainWindow) {
      final mainWindows = Platform.plat().mainWindows;
      for (var mw in mainWindows) {
        windowWidgets.add(WindowWidget.fromView(mw.view()));
      }
    }

    final floatingWindows = Platform.plat().floatingWindows;
    for (var fw in floatingWindows) {
      final windowWidget =
          (KDDWBindingsCore.View.fromCache(fw.view().thisCpp) as View_mixin)
              .windowWidget;
      windowWidgets.add(windowWidget!);
    }

    final dropIndicatorWindows = Platform.plat().indicatorWindows;
    for (var w in dropIndicatorWindows) {
      if (w.isVisible()) windowWidgets.add(WindowWidget.fromView(w));
    }

    return Stack(children: windowWidgets);
  }

  onWindowCountChanged() {
    setState(() {});
  }
}
