/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'dart:developer';

import 'package:KDDockWidgets/MainWindow.dart';
import 'package:KDDockWidgets/View_mixin.dart';
import 'package:KDDockWidgets/View.dart';
import 'package:KDDockWidgets/WindowOverlayWidget.dart';
import 'package:KDDockWidgetsBindings/Bindings.dart' as KDDockWidgetBindings;
import 'package:KDDockWidgetsBindings/Bindings.dart';
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsCore.dart'
    as KDDWBindingsCore;
import 'package:KDDockWidgetsBindings/Bindings_KDDWBindingsFlutter.dart'
    as KDDWBindingsFlutter;
import 'ViewFactory.dart';

class Platform extends KDDWBindingsFlutter.Platform {
  late final WindowOverlayWidget windowOverlayWidget;

  var floatingWindows = <KDDWBindingsCore.FloatingWindow>[];
  var mainWindows = <KDDWBindingsCore.MainWindow>[];
  var indicatorWindows = <KDDWBindingsFlutter.IndicatorWindow>[];

  @override
  @pragma("vm:entry-point")
  String name() {
    return "flutter";
  }

  static Platform plat() {
    var p = KDDWBindingsFlutter.Platform.platformFlutter();

    return KDDWBindingsFlutter.Platform.fromCache(p.thisCpp) as Platform;
  }

  @override
  @pragma("vm:entry-point")
  KDDWBindingsCore.ViewFactory createDefaultViewFactory() {
    try {
      return ViewFactory();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  KDDWBindingsCore.View tests_createView(
      KDDockWidgetBindings.CreateViewOptions opts,
      {required KDDWBindingsCore.View? parent}) {
    try {
      final view = View(null, 0, parent);

      view.setMinimumSize(opts.getMinSize());
      view.setMaximumSize(opts.getMaxSize());
      view.setVisible(opts.isVisible);

      return view;
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  KDDWBindingsCore.View createView(KDDWBindingsCore.Controller? controller,
      {required KDDWBindingsCore.View? parent}) {
    try {
      return GenericView(controller, parent);
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  KDDWBindingsCore.MainWindow createMainWindow(
      String? uniqueName, CreateViewOptions viewOpts,
      {int options =
          KDDockWidgets_MainWindowOption.MainWindowOption_HasCentralFrame,
      required KDDWBindingsCore.View? parent,
      int arg__5 = 0}) {
    try {
      final parentView =
          parent == null ? null : (parent as KDDWBindingsFlutter.MainWindow);
      var mw = MainWindow(uniqueName,
          parent: parentView, flags: arg__5, options: options);

      mw.resize(viewOpts.getSize());

      return mw.m_controller;
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  onFloatingWindowCreated(KDDWBindingsCore.FloatingWindow? fw) {
    try {
      floatingWindows.add(fw!);
      rebuildWindowOverlay();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  onFloatingWindowDestroyed(KDDWBindingsCore.FloatingWindow? fw) {
    try {
      final oldSize = floatingWindows.length;
      floatingWindows.removeWhere((it) => it.thisCpp == fw!.thisCpp);
      // kddw emits windowDestroyed twice, one when it's scheduled and one in dtor
      // so only rebuild if list length actually changed
      if (oldSize != floatingWindows.length) {
        rebuildWindowOverlay();
      }
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  onMainWindowCreated(KDDWBindingsCore.MainWindow? mw) {
    try {
      mainWindows.add(mw!);
      rebuildWindowOverlay();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  onMainWindowDestroyed(KDDWBindingsCore.MainWindow? mw) {
    try {
      final oldSize = mainWindows.length;
      mainWindows.removeWhere((it) => it.thisCpp == mw!.thisCpp);
      if (oldSize != mainWindows.length) rebuildWindowOverlay();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  onDropIndicatorOverlayCreated(KDDWBindingsFlutter.IndicatorWindow? w) {
    try {
      indicatorWindows.add(w!);
      rebuildWindowOverlay();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  onDropIndicatorOverlayDestroyed(KDDWBindingsFlutter.IndicatorWindow? w) {
    try {
      final oldSize = indicatorWindows.length;
      indicatorWindows.removeWhere((it) => it.thisCpp == w!.thisCpp);
      if (oldSize != indicatorWindows.length) rebuildWindowOverlay();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  void raiseFloatingWindow(KDDWBindingsCore.FloatingWindow fw) {
    final int oldSize = floatingWindows.length;

    floatingWindows.removeWhere((it) => it.thisCpp == fw.thisCpp);
    if (oldSize == floatingWindows.length) {
      print("raiseFloatingWindow: Failed to find floating window");
    } else {
      floatingWindows.add(fw);
      rebuildWindowOverlay();
    }
  }

  @override
  @pragma("vm:entry-point")
  void rebuildWindowOverlay() {
    try {
      WindowOverlayWidget.globalKey().currentState?.onWindowCountChanged();
    } on Exception catch (e) {
      print("Exception $e");
      throw e;
    }
  }

  @override
  @pragma("vm:entry-point")
  void dumpManagedBacktrace() {
    print(StackTrace.current);
  }

  @override
  @pragma("vm:entry-point")
  void pauseForDartDebugger() {
    debugger();
  }

  @override
  @pragma("vm:entry-point")
  void runDelayed(int ms, KDDWBindingsCore.DelayedCall? c) {
    if (c != null)
      Future.delayed(Duration(milliseconds: ms), () {
        c.call();
        c.release();
      });
  }

  @override
  @pragma("vm:entry-point")
  void scheduleResumeCoRoutines(int ms) {
    Future.delayed(Duration(milliseconds: ms), () {
      resumeCoRoutines();
    });
  }
}

class GenericView extends KDDWBindingsFlutter.View with View_mixin {
  GenericView(
      KDDWBindingsCore.Controller? controller, KDDWBindingsCore.View? parent)
      : super(controller, 0, parent) {
    m_fillsParent = true;
    initMixin(this, debugName: "GenericView", parent: parent);
  }
}
