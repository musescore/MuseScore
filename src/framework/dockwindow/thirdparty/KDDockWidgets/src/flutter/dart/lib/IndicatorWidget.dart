/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings.dart';
import 'package:flutter/material.dart';
import 'package:flutter/material.dart' as Geometry show Rect;
import 'dart:ffi' as ffi;
import 'IndicatorWindow.dart';
import 'GlobalStringKey.dart';

/// Represents a single drop indicator

class IndicatorWidget extends StatefulWidget {
  final int loc;
  final IndicatorWindow indicatorWindow;
  IndicatorWidget(this.indicatorWindow, this.loc)
      : super(key: indicatorKey(indicatorWindow, loc)) {}

  static GlobalStringKey indicatorKey(
      IndicatorWindow indicatorWindow, int loc) {
    final ffi.Pointer<ffi.Void> ptr =
        indicatorWindow.kddwView.thisCpp.cast<ffi.Void>();

    return GlobalStringKey("IndicatorWidget-${ptr.address}-${loc}");
  }

  @override
  State<IndicatorWidget> createState() => _IndicatorWidgetState(loc);

  _IndicatorWidgetState? widgetState() {
    final globalKey = key as GlobalStringKey;
    return globalKey.currentState as _IndicatorWidgetState?;
  }

  Offset globalPosOfCenter() {
    final globalKey = key as GlobalStringKey;
    final renderBox =
        globalKey.currentContext?.findRenderObject() as RenderBox?;
    if (renderBox == null) {
      print("globalPosOfCenter(): No renderbox found");
      return Offset.zero;
    }

    final localPos =
        Offset(renderBox.size.width / 2.0, renderBox.size.height / 2.0);

    return renderBox.localToGlobal(localPos);
  }

  int hover(Offset pt) {
    return widgetState()?.hover(pt) ??
        KDDockWidgets_DropLocation.DropLocation_None;
  }

  void updatePosition(int overlayWidth, int overlayHeight,
      KDDWBindingsCore.Group? hoveredGroup, int visibleLocations) {
    final globalKey = key as GlobalStringKey;
    final state = widgetState();

    if (state == null) {
      print("Null state for key $globalKey");
    } else {
      state.updatePosition(
          overlayWidth, overlayHeight, hoveredGroup, visibleLocations);
    }
  }
}

String filenameForLocationType(int loc) {
  switch (loc) {
    case KDDockWidgets_DropLocation.DropLocation_Left:
      return "inner_left";
    case KDDockWidgets_DropLocation.DropLocation_Top:
      return "inner_top";
    case KDDockWidgets_DropLocation.DropLocation_Right:
      return "inner_right";
    case KDDockWidgets_DropLocation.DropLocation_Bottom:
      return "inner_bottom";
    case KDDockWidgets_DropLocation.DropLocation_Center:
      return "center";
    case KDDockWidgets_DropLocation.DropLocation_OutterLeft:
      return "outter_left";
    case KDDockWidgets_DropLocation.DropLocation_OutterTop:
      return "outter_top";
    case KDDockWidgets_DropLocation.DropLocation_OutterRight:
      return "outter_right";
    case KDDockWidgets_DropLocation.DropLocation_OutterBottom:
      return "outter_bottom";
  }
  return "";
}

class _IndicatorWidgetState extends State<IndicatorWidget> {
  final int loc;
  bool active = false;
  double x = 0;
  double y = 0;
  final length = 50.0;
  get halfLength => length / 2;
  bool visible = false;

  String filename() {
    var name = filenameForLocationType(loc);
    if (active) name += "_active";
    return "packages/KDDockWidgets/assets/classic_indicators/${name}.png";
  }

  void setActive(bool a) {
    if (active != a) {
      setState(() {
        active = a;
      });
    }
  }

  _IndicatorWidgetState(this.loc);

  @override
  Widget build(BuildContext context) {
    if (!visible) return Container(color: Colors.transparent);
    return Positioned(
        left: x,
        top: y,
        width: length,
        height: length,
        child: Image.asset(filename()));
  }

  Offset _positionForInnerLocation(KDDWBindingsCore.Group? hoveredGroup) {
    if (hoveredGroup == null) return Offset.zero;

    Rect groupGeo = hoveredGroup.geometry();
    var x = groupGeo.x().toDouble();
    var y = groupGeo.y().toDouble();
    var width = groupGeo.width().toDouble();
    var height = groupGeo.height().toDouble();

    final spacing = 10.0;
    final center =
        Offset(x + width / 2 - halfLength, y + height / 2 - halfLength);

    switch (loc) {
      case KDDockWidgets_DropLocation.DropLocation_Left:
        return center + Offset(-length - spacing, 0);
      case KDDockWidgets_DropLocation.DropLocation_Top:
        return center + Offset(0, -length - spacing);
      case KDDockWidgets_DropLocation.DropLocation_Right:
        return center + Offset(length + spacing, 0);
      case KDDockWidgets_DropLocation.DropLocation_Bottom:
        return center + Offset(0, length + spacing);

      case KDDockWidgets_DropLocation.DropLocation_Center:
        return center;
    }

    return Offset.zero;
  }

  Offset _positionForLocation(double overlayWidth, double overlayHeight,
      KDDWBindingsCore.Group? hoveredGroup) {
    switch (loc) {
      case KDDockWidgets_DropLocation.DropLocation_Left:
      case KDDockWidgets_DropLocation.DropLocation_Top:
      case KDDockWidgets_DropLocation.DropLocation_Right:
      case KDDockWidgets_DropLocation.DropLocation_Bottom:
      case KDDockWidgets_DropLocation.DropLocation_Center:
        return _positionForInnerLocation(hoveredGroup);
      case KDDockWidgets_DropLocation.DropLocation_OutterLeft:
        return Offset(halfLength, overlayHeight / 2 - halfLength);
      case KDDockWidgets_DropLocation.DropLocation_OutterTop:
        return Offset(overlayWidth / 2 - halfLength, halfLength);
      case KDDockWidgets_DropLocation.DropLocation_OutterRight:
        return Offset(
            overlayWidth - 3 * halfLength, overlayHeight / 2 - halfLength);
      case KDDockWidgets_DropLocation.DropLocation_OutterBottom:
        return Offset(
            overlayWidth / 2 - halfLength, overlayHeight - 3 * halfLength);
    }
    return Offset(0, 0);
  }

  /// Returns this indicator's rect, in coordinates of parent (i.e. drop indicator overlay coordinates)
  Geometry.Rect geometry() {
    return Geometry.Rect.fromLTWH(x, y, length, length);
  }

  int hover(Offset pt) {
    if (!visible) return KDDockWidgets_DropLocation.DropLocation_None;

    if (geometry().contains(pt)) {
      setActive(true);
      return loc;
    } else {
      setActive(false);
      return KDDockWidgets_DropLocation.DropLocation_None;
    }
  }

  void updatePosition(int overlayWidth, int overlayHeight,
      KDDWBindingsCore.Group? hoveredGroup, int visibleLocations) {
    final newPos = _positionForLocation(
        overlayWidth.toDouble(), overlayHeight.toDouble(), hoveredGroup);
    final newVisibility = (visibleLocations & loc) != 0;
    if (newPos != Offset(x, y) || visible != newVisibility) {
      setState(() {
        x = newPos.dx;
        y = newPos.dy;
        visible = newVisibility;
      });
    }
  }
}
