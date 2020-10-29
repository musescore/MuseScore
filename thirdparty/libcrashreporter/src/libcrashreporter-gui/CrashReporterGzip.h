/*
 * libcrashreporter-qt
 *
 * Copyright (C) 2010-2011  Christian Muehlhaeuser <muesli@tomahawk-player.org>
 * Copyright (C) 2015-2016  Teo Mrnjavac <teo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRASHREPORTER_GZIP_H
#define CRASHREPORTER_GZIP_H

#include <QDataStream>

QByteArray gzip_compress( const QByteArray& data );

#endif // CRASHREPORTER_GZIP_H
