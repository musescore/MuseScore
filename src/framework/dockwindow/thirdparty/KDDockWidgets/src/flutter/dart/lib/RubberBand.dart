/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/PositionedWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings.dart' as KDDockWidgetBindings;
import 'package:KDDockWidgetsBindings/Bindings.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;
import 'package:flutter/gestures.dart';

import 'package:flutter/material.dart' hide View;

class RubberBand extends KDDWBindingsFlutter.View with View_mixin {
  RubberBand(KDDWBindingsCore.View? parent)
      : super(null, KDDockWidgetBindings.Core_ViewType.RubberBand, parent) {
    initMixin(this, debugName: "RubberBand", parent: parent);
  }

  PositionedWidget createFlutterWidget() {
    return RubberBandWidget(this, widgetKey);
  }
}

class RubberBandWidget extends PositionedWidget {
  RubberBandWidget(view, Key key) : super(view, key: key);

  @override
  State<PositionedWidget> createState() {
    return RubberBandPositionedWidgetState(kddwView);
  }
}

class RubberBandPositionedWidgetState extends PositionedWidgetState {
  RubberBandPositionedWidgetState(view) : super(view);

  @override
  Widget buildContents(BuildContext ctx) {
    return Container(
      decoration: new BoxDecoration(
          borderRadius: new BorderRadius.circular(1.0),
          color: Color(0x555ca1c5),
          border: Border.all(
            color: Color.fromARGB(255, 39, 77, 97),
            width: 2,
          )),
    );
  }
}
