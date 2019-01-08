//=============================================================================
//  FilterableView
//
//  Copyright (C) 2018 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "filterableview.h"
#include "../stringutils.h"

namespace Ms {

//---------------------------------------------------------
//   FilterableView (pure virtual class has no constructor)
//---------------------------------------------------------

//---------------------------------------------------------
//   matches
// Does the node text contain the search string?
//---------------------------------------------------------

bool FilterableView::matches(const QModelIndex& node, const QString& searchString)
      {
      // replace the unicode b (accidental) so a search phrase of "bb" would match "Bb Trumpet", etc
      QString text = node.data().toString().replace(QChar(0x266d), QChar('b'));

      if (text.contains(searchString, Qt::CaseInsensitive))
            return true;

      // replace special characters
      text = stringutils::removeLigatures(text);
      text = stringutils::removeDiacritics(text);

      return text.contains(searchString, Qt::CaseInsensitive);
      }

}
