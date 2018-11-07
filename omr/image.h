//=============================================================================
//  MusE Reader
//  Music Score Reader
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

namespace Ms {

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

class Image : public QImage {
   public:
      };

extern double imageSkew(const QImage& image);

}

#endif

