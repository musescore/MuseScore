/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/PositionedWidget.dart';
import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;
import 'package:flutter/material.dart' hide View;

class DockWidget extends KDDWBindingsFlutter.DockWidget with View_mixin {
  Widget? guestwidget;
  DockWidget(String? uniqueName, {int options = 0, int layoutSaverOptions = 0})
      : super(uniqueName,
            options: options, layoutSaverOptions: layoutSaverOptions) {
    m_fillsParent = true;
    initMixin(this, color: Colors.pink, debugName: "DockWidget", parent: null);
  }

  void setGuestWidget(Widget w) {
    guestwidget = w;
    widgetState<DockWidgetPositionedWidgetState>()?.setGuestWidget(w);
  }

  PositionedWidget createFlutterWidget() {
    return DockWidgetWidget(this, widgetKey);
  }
}

class DockWidgetWidget extends PositionedWidget {
  DockWidgetWidget(View_mixin kddwView, Key key) : super(kddwView, key: key);

  @override
  State<PositionedWidget> createState() {
    return DockWidgetPositionedWidgetState(kddwView);
  }
}

class DockWidgetPositionedWidgetState extends PositionedWidgetState {
  DockWidgetPositionedWidgetState(var kddwView) : super(kddwView) {}

  void setGuestWidget(Widget w) {
    setState(() {});
  }

  @override
  Widget buildContents(BuildContext ctx) {
    final DockWidget dockWidgetView = kddwView as DockWidget;

    if (dockWidgetView.guestwidget == null) {
      return Container();
    } else {
      return dockWidgetView.guestwidget!;
    }
  }
}
