//=============================================================================
//  MuseScore
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
//=============================================================================

#ifndef PREFERENCESTREEWIDGET_DELEGATE_H
#define PREFERENCESTREEWIDGET_DELEGATE_H

namespace Ms {

// This class is needed to increase the default row height of the PreferencesListWidget.
class PreferencesTreeWidget_Delegate : public QItemDelegate
{
   public:
      PreferencesTreeWidget_Delegate(QObject* parent = nullptr);

      QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const;

}; // class PreferencesTreeWidget_Delegate

} // Ms

#endif // PREFERENCESTREEWIDGET_DELEGATE_H
