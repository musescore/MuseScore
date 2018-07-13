/*******************************************************************************
//  MusEScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*******************************************************************************/

#include "preferencestreewidget_delegate.h"

namespace Ms {

PreferencesTreeWidget_Delegate::PreferencesTreeWidget_Delegate(QObject* parent)
      : QItemDelegate(parent)
      {
      setObjectName("PreferencesTreeWidget_Delegate");
      }

QSize PreferencesTreeWidget_Delegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
      {
      return QSize(160, 24);
      }

} // Ms
