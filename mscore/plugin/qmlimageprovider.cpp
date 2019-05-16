//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#include "qmlimageprovider.h"

#include "musescore.h"
#include "icons.h"

namespace Ms {

QPixmap ScoreThumbnailProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
      {
      QPixmap pixmap = mscore->extractThumbnail(id);

      if (pixmap.isNull())
            pixmap = icons[int(Icons::file_ICON)]->pixmap(QSize(50,60));

      if (requestedSize.isValid())
            pixmap = pixmap.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

      (*size) = pixmap.size();
      return pixmap;
      }

} // namespace Ms
