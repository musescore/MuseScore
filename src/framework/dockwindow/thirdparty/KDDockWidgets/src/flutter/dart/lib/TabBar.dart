/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/PositionedWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart' hide View;
import 'package:flutter/material.dart' as material show TabBar;
import 'package:flutter/material.dart' as Geometry show Rect, Size;
import 'dart:ui';

class TabBar extends KDDWBindingsFlutter.TabBar with View_mixin {
  late final KDDWBindingsCore.TabBar m_controller;

  TabBar(KDDWBindingsCore.TabBar? tabBar, KDDWBindingsCore.View? parent)
      : super(tabBar, parent: parent) {
    m_fillsParent = true;
    m_controller = tabBar!;
    initMixin(this, parent: parent);
  }

  PositionedWidget createFlutterWidget() {
    return TabBarWidget(this, widgetKey);
  }

  void tabsForTabBar(RenderBox rb, List<RenderBox> result) {
    rb.visitChildren((child) {
      final rbChild = child as RenderBox?;
      if (rbChild != null) {
        if (rbChild.runtimeType.toString() == "_TabLabelBarRenderer") {
          rbChild.visitChildren((child) {
            final tab = child as RenderBox?;
            if (tab != null) {
              result.add(tab);
            }
          });
        } else {
          tabsForTabBar(rbChild, result);
        }
      }
    });
  }

  void dumpRenderBox(RenderBox rb, int level) {
    print(
        "${"".padLeft(level * 4)} $rb size=${rb.size} type=${rb.runtimeType.toString()}");

    rb.visitChildren((child) {
      final rbChild = child as RenderBox?;
      if (rbChild != null) {
        dumpRenderBox(rbChild, level + 1);
      }
    });
  }

  @override
  int tabAt(Point localPos) {
    final localX = localPos.x().toDouble();
    final localY = localPos.y().toDouble();

    final renderBox = widgetKey.currentContext?.findRenderObject() as RenderBox;
    final windowCoords = renderBox.localToGlobal(Offset(localX, localY));

    List<RenderBox> tabs = [];
    tabsForTabBar(renderBox, tabs);

    for (int i = 0; i < tabs.length; ++i) {
      final tab = tabs[i];
      final tabWindowPos = tab.localToGlobal(Offset.zero);

      final tabWindowRect = Geometry.Rect.fromLTWH(
          tabWindowPos.dx, tabWindowPos.dy, tab.size.width, tab.size.height);

      if (tabWindowRect.contains(windowCoords)) return i;
    }

    return -1;
  }
}

class TabBarWidget extends PositionedWidget {
  TabBarWidget(View_mixin kddwView, Key key) : super(kddwView, key: key);

  @override
  State<PositionedWidget> createState() {
    return TabBarPositionedWidgetState(kddwView);
  }
}

class TabBarPositionedWidgetState extends PositionedWidgetState {
  int _currentIndex = -1;
  bool _calledFromFlutter = false;

  TabBar tabBarView() {
    return kddwView as TabBar;
  }

  set currentIndex(int index) {
    if (index != _currentIndex) {
      _currentIndex = index;

      if (_calledFromFlutter) {
        // User clicked a tab, tell C++
        tabBarView().m_controller.setCurrentIndex(index);
      } else {
        // C++ requested it, rebuild
        // FLUTTER_TODO: ask controller ?
        setState(() {});
      }
    }
  }

  TabBarPositionedWidgetState(var kddwView) : super(kddwView);

  @override
  Widget buildContents(BuildContext ctx) {
    final tabBarView = this.tabBarView();
    final int numTabs = tabBarView.m_controller.numDockWidgets();
    final tabHeight = 40.0;
    final double tabWidth = 120;

    if (numTabs == 0) return Container();

    final tabs = <Widget>[];
    for (var i = 0; i < numTabs; ++i) {
      final dw = tabBarView.m_controller.dockWidgetAt_2(i);
      tabs.add(MyTab(
          height: tabHeight,
          child: Text("${dw.title().toDartString()}",
              style: Theme.of(context).textTheme.bodyMedium)));
    }

    final curIndex = tabBarView.m_controller.currentIndex();
    if (curIndex == -1) {
      print("ERROR: Expected curIndex >= 0");
    }

    return Listener(
        onPointerDown: (event) {
          kddwView.onFlutterMouseEvent(event);
        },
        onPointerUp: (event) {
          kddwView.onFlutterMouseEvent(event);
        },
        onPointerMove: (event) {
          if (event.buttons != kPrimaryButton) return;
          kddwView.onFlutterMouseEvent(event);
        },
        child: Align(
          alignment: Alignment.topLeft,
          child: SizedBox(
              height: tabHeight,
              width: tabWidth * numTabs,
              child: DefaultTabController(
                  initialIndex: curIndex == -1 ? 0 : curIndex,
                  length: numTabs,
                  child: Builder(
                    builder: (context) {
                      final TabController controller =
                          DefaultTabController.of(context);
                      controller.addListener(() {
                        if (!controller.indexIsChanging) {
                          _calledFromFlutter = true;
                          currentIndex = controller.index;
                          _calledFromFlutter = false;
                        }
                      });

                      return material.TabBar(
                        tabs: tabs,
                        splashBorderRadius: BorderRadius.circular(8),
                        indicatorSize: TabBarIndicatorSize.label,
                      );
                    },
                  ))),
        ));
  }
}

class MyTab extends StatelessWidget implements PreferredSizeWidget {
  MyTab({
    super.key,
    required this.child,
    required this.height,
  });

  final Widget child;
  final double height;
  final isHovering = ValueNotifier(false);

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      onHover: (event) {
        if (isHovering.value) return;
        isHovering.value = true;
      },
      onExit: (event) {
        if (!isHovering.value) return;
        isHovering.value = false;
      },
      child: ValueListenableBuilder(
        valueListenable: isHovering,
        builder: (context, isHovering, widget) {
          return SizedBox(
            height: height,
            child: isHovering
                ? Stack(
                    children: [
                      Align(
                        child: child,
                      ),
                      Align(
                        alignment: Alignment.centerRight,
                        child: Icon(
                          Icons.open_with,
                          size: 18,
                          color: Theme.of(context)
                              .colorScheme
                              .onSurfaceVariant
                              .withAlpha(100),
                        ),
                      ),
                    ],
                  )
                : Center(
                    widthFactor: 1.5,
                    child: child,
                  ),
          );
        },
      ),
    );
  }

  @override
  Geometry.Size get preferredSize {
    return Geometry.Size.fromHeight(height);
  }
}
