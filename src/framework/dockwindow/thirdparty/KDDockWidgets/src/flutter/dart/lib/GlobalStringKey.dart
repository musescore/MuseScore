/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import 'package:flutter/widgets.dart';

/// A GlobalObjectKey that uses String.operator== for comparison
/// instead of relying on object identity, which doesn't always work for
/// equal strings

class GlobalStringKey<T extends State<StatefulWidget>>
    extends GlobalObjectKey<T> {
  GlobalStringKey(String value) : super(value);

  @override
  bool operator ==(other) {
    if (other is GlobalStringKey) {
      return super.value == other.value;
    }
    return false;
  }

  @override
  int get hashCode => super.value.hashCode;
}
