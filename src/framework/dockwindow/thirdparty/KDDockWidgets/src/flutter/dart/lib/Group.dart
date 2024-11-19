/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/PositionedWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;

import 'package:flutter/material.dart' hide View;

import 'DockWidget.dart';
import 'TabBar.dart' as kddw;
import 'TitleBar.dart';

class Group extends KDDWBindingsFlutter.Group with View_mixin {
  late final KDDWBindingsCore.Group m_controller;

  Group(KDDWBindingsCore.Group? group, KDDWBindingsCore.View? parent)
      : super(group, parent: parent) {
    m_controller = group!;
    initMixin(this,
        color: Colors.greenAccent, debugName: "Group", parent: parent);
  }

  PositionedWidget createFlutterWidget() {
    return GroupWidget(this, widgetKey);
  }

  TitleBar titleBarView() {
    return KDDWBindingsFlutter.View.fromCache(
        m_controller.titleBar().view().thisCpp) as TitleBar;
  }

  kddw.TabBar tabBarView() {
    return KDDWBindingsFlutter.View.fromCache(
        m_controller.stack().tabBar().view().thisCpp) as kddw.TabBar;
  }

  DockWidget? dockWidgetView() {
    final dw = m_controller.currentDockWidget();
    if (dw.thisCpp.address != 0) // Add "isNullptr"
      return KDDWBindingsFlutter.View.fromCache(dw.view().thisCpp)
          as DockWidget;

    if (!m_controller.isCentralGroup())
      print("Group: No dock widget in the Group!");
    return null;
  }
}

class GroupWidget extends PositionedWidget {
  GroupWidget(var kddwView, Key key) : super(kddwView, key: key);

  @override
  State<PositionedWidget> createState() {
    return GroupPositionedWidgetState(kddwView);
  }
}

class GroupPositionedWidgetState extends PositionedWidgetState {
  GroupPositionedWidgetState(var kddwView) : super(kddwView);

  @override
  Widget buildContents(BuildContext ctx) {
    final Group groupView = kddwView as Group;
    final titleBarView = groupView.titleBarView();
    final tabBarView = groupView.tabBarView();
    final dockWidgetWidget = groupView.dockWidgetView()?.flutterWidget ??
        Container(color: Colors.black);
    final group = groupView.m_controller;

    /// FLUTTER_TODO: Move this logic to C++
    final bool showTabBar = !tabBarView.isExplicitlyHidden() &&
        (group.alwaysShowsTabs() || group.dockWidgetCount() > 1);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        if (!titleBarView.isExplicitlyHidden()) titleBarView.flutterWidget,
        if (showTabBar) tabBarView.flutterWidget,
        Expanded(child: dockWidgetWidget)
      ],
    );
  }
}
