/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/View_mixin.dart';
import 'package:flutter/material.dart' hide View;

/// A widget that supports being positioned. It's child of a Stack widget.
class PositionedWidget extends StatefulWidget {
  final View_mixin kddwView;

  PositionedWidget(this.kddwView, {Key? key}) : super(key: key);

  @override
  State<PositionedWidget> createState() {
    return PositionedWidgetState(kddwView);
  }
}

class PositionedWidgetState extends State<PositionedWidget>
    with WidgetsBindingObserver {
  View_mixin kddwView;
  final bool _fillsParent;

  PositionedWidgetState(this.kddwView) : _fillsParent = kddwView.m_fillsParent;

  @override
  void initState() {
    WidgetsBinding.instance.addObserver(this);
    super.initState();
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeMetrics() {
    // Whenever the window resizes we rebuild
    if (_fillsParent) {
      // setState(() {});
      resizeKDDWLayout();
    }
  }

  /// Called when children were added or removed (or hidden).
  void childrenChanged() {
    _updateState();
  }

  /// Called when the KDDW View changes size. Rebuilds the flutter Widget.
  void updatePosition() {
    _updateState();
  }

  /// Called when the KDDW View changes size. Rebuilds the flutter Widget.
  void updateSize() {
    _updateState();
  }

  void _updateState() {
    // print("_updateState $this");
    setState(() {});
  }

  void rebuildRequested() {
    _updateState();
  }

  void resizeKDDWLayout() {
    // If the widget is resized by flutter then tell KDDW.
    // Example use case: User resizes the window, which resizes the KDDW layout
    if (!_fillsParent) return;

    final renderBox =
        kddwView.widgetKey.currentContext?.findRenderObject() as RenderBox;

    final Size size = renderBox.size;
    final geo = kddwView.viewGeometry();
    if (size.width != geo.width() || size.height != geo.height()) {
      kddwView.kddwView
          .onFlutterWidgetResized(size.width.toInt(), size.height.toInt());
    }
  }

  /// This is factored-out from build() so derived classes can return something else
  Widget buildContents(BuildContext ctx) {
    return Container(color: kddwView.m_color);
  }

  @override
  Widget build(BuildContext ctx) {
    WidgetsBinding.instance.addPostFrameCallback((_) {
      resizeKDDWLayout();
    });

    final container = buildContents(ctx);
    if (_fillsParent) return container;

    // FLUTTER_TODO: Pass whole struct in one go, minimize ffi calls
    final geo = kddwView.viewGeometry();

    return Positioned(
        width: geo.width() * 1.0,
        height: geo.height() * 1.0,
        top: geo.y() * 1.0,
        left: geo.x() * 1.0,
        child: container);
  }
}
