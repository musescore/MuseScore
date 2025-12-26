# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(iex_imagesexport)

target_sources(iex_imagesexport PRIVATE
    imagesexportmodule.cpp
    imagesexportmodule.h
    iimagesexportconfiguration.h
    internal/imagesexportconfiguration.cpp
    internal/imagesexportconfiguration.h

    internal/abstractimagewriter.cpp
    internal/abstractimagewriter.h
    internal/svgwriter.cpp
    internal/svgwriter.h
    internal/svggenerator.cpp
    internal/svggenerator.h
    internal/pngwriter.cpp
    internal/pngwriter.h
    internal/pdfwriter.cpp
    internal/pdfwriter.h
)

target_link_libraries(iex_imagesexport PRIVATE engraving)
