/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/IndicatorWidget.dart';
import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/PositionedWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings.dart' as KDDockWidgetBindings;
import 'package:KDDockWidgetsBindings/Bindings.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;
import 'package:flutter/material.dart' hide View;

class IndicatorWindow extends KDDWBindingsFlutter.IndicatorWindow
    with View_mixin {
  IndicatorWindow(
      KDDWBindingsCore.ClassicDropIndicatorOverlay? classicIndicators,
      KDDWBindingsCore.View? parent)
      : super(classicIndicators, parent) {
    m_fillsParent = true;
    initMixin(this, debugName: "IndicatorWindow", parent: parent);
  }

  PositionedWidget createFlutterWidget() {
    return IndicatorWindowWidget(this, widgetKey);
  }

  @override
  @pragma("vm:entry-point")
  bool updatePositions_flutter(int overlayWidth, int overlayHeight,
      KDDWBindingsCore.Group? hoveredGroup, int visibleLocations) {
    try {
      final state = widgetState<IndicatorWindowWidgetState>();
      if (state == null) return false;
      state.updatePositions(
          overlayWidth, overlayHeight, hoveredGroup, visibleLocations);
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }

    return true;
  }

  @override
  @pragma("vm:entry-point")
  int hover_flutter(Point pt) {
    // FLUTTER_TODO: Receive 2 doubles, less allocations and ffi
    try {
      final localPt = Offset(pt.x().toDouble(), pt.y().toDouble());
      return widgetState<IndicatorWindowWidgetState>()?.hover(localPt) ?? 0;
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  Point posForIndicator_flutter(int droploc) {
    try {
      return widgetState<IndicatorWindowWidgetState>()
              ?.posForIndicator(droploc) ??
          Point.ctor2(0, 0);
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }
}

class IndicatorWindowWidget extends PositionedWidget {
  IndicatorWindowWidget(View_mixin view, Key key) : super(view, key: key);

  @override
  State<PositionedWidget> createState() {
    return IndicatorWindowWidgetState(kddwView);
  }
}

class IndicatorWindowWidgetState extends PositionedWidgetState {
  final List<IndicatorWidget> indicatorWidgets = [];
  final List<int> locations = [
    KDDockWidgets_DropLocation.DropLocation_Left,
    KDDockWidgets_DropLocation.DropLocation_Top,
    KDDockWidgets_DropLocation.DropLocation_Right,
    KDDockWidgets_DropLocation.DropLocation_Bottom,
    KDDockWidgets_DropLocation.DropLocation_Center,
    KDDockWidgets_DropLocation.DropLocation_OutterLeft,
    KDDockWidgets_DropLocation.DropLocation_OutterTop,
    KDDockWidgets_DropLocation.DropLocation_OutterRight,
    KDDockWidgets_DropLocation.DropLocation_OutterBottom
  ];

  IndicatorWindow indicatorWindow() {
    return kddwView as IndicatorWindow;
  }

  IndicatorWindowWidgetState(View_mixin indicatorWindow)
      : super(indicatorWindow) {
    IndicatorWindow w = this.indicatorWindow();
    for (var loc in locations) {
      indicatorWidgets.add(IndicatorWidget(w, loc));
    }
  }

  @override
  Widget buildContents(BuildContext ctx) {
    return Stack(
      children: indicatorWidgets,
    );
  }

  void updatePositions(int overlayWidth, int overlayHeight,
      KDDWBindingsCore.Group? hoveredGroup, int visibleLocations) {
    for (var widget in indicatorWidgets)
      widget.updatePosition(
          overlayWidth, overlayHeight, hoveredGroup, visibleLocations);
  }

  int hover(Offset localPt) {
    int result = KDDockWidgets_DropLocation.DropLocation_None;
    for (var indicator in indicatorWidgets) {
      final loc = indicator.hover(localPt);
      // don't break once found, so others can update icon
      if (loc != KDDockWidgets_DropLocation.DropLocation_None) result = loc;
    }

    return result;
  }

  IndicatorWidget indicatorWidgetForLoc(int loc) {
    return indicatorWidgets.firstWhere((w) => w.loc == loc);
  }

  Point posForIndicator(int droploc) {
    IndicatorWidget indicator = indicatorWidgetForLoc(droploc);
    final Offset globalPos = indicator.globalPosOfCenter();
    return Point.ctor2(globalPos.dx.toInt(), globalPos.dy.toInt());
  }
}
