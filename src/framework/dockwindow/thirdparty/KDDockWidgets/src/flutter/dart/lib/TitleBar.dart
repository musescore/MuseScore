/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/PositionedWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings.dart' as KDDockWidgetBindings;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;
import 'package:flutter/gestures.dart';

import 'package:flutter/material.dart' hide View;

class TitleBar extends KDDWBindingsFlutter.TitleBar with View_mixin {
  TitleBar(KDDWBindingsCore.TitleBar? titleBar, KDDWBindingsCore.View? parent)
      : super(titleBar, parent: parent) {
    m_fillsParent = true;
    initMixin(this, debugName: "TitleBar", parent: parent);
  }

  @override
  void onTitleBarChanged(String? newTitle) {
    widgetState<TitleBarPositionedWidgetState>()?.title = newTitle!;
  }

  PositionedWidget createFlutterWidget() {
    return TitleBarWidget(this, widgetKey);
  }
}

class TitleBarWidget extends PositionedWidget {
  TitleBarWidget(View_mixin kddwView, Key key) : super(kddwView, key: key);

  @override
  State<PositionedWidget> createState() {
    return TitleBarPositionedWidgetState(kddwView);
  }
}

class TitleBarPositionedWidgetState extends PositionedWidgetState {
  String? _title;
  set title(String newTitle) {
    if (_title != newTitle) {
      setState(() {
        _title = newTitle;
      });
    }
  }

  TitleBarPositionedWidgetState(var kddwView) : super(kddwView);

  @override
  Widget buildContents(BuildContext ctx) {
    final titleBarView = kddwView as TitleBar;
    const double vPadding = 4;
    const double iconVPadding = 4;
    const double maxIconHeight = 26;
    final ButtonStyle style = IconButton.styleFrom(
      foregroundColor: Theme.of(context).colorScheme.onSurfaceVariant,
      minimumSize: const Size(26, maxIconHeight),
      maximumSize: const Size(26, maxIconHeight),
      padding: const EdgeInsets.all(iconVPadding),
      iconSize: 22,
    );

    final contents = SizedBox(
      height: maxIconHeight + (2 * vPadding) + (2 * iconVPadding),
      child: Container(
        color: Colors.transparent, // for hit testing
        padding: const EdgeInsets.only(
            top: vPadding, bottom: vPadding, left: 16, right: 4),
        child: Row(
          children: [
            Expanded(
              child: Text(
                titleBarView.asTitleBarController().title().toDartString(),
                style: Theme.of(context).textTheme.bodyMedium,
              ),
            ),
            IconButton(
              onPressed: () {
                titleBarView.asTitleBarController().onFloatClicked();
              },
              style: style,
              icon: const Icon(
                Icons.filter_none,
              ),
            ),
            IconButton(
              onPressed: () {
                titleBarView.asTitleBarController().onCloseClicked();
              },
              style: style,
              icon: const Icon(
                Icons.close,
              ),
            ),
          ],
        ),
      ),
    );

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
        child: contents);
  }
}
