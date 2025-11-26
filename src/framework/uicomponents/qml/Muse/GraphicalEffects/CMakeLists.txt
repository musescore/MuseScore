# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

muse_create_qml_module(muse_graphicaleffects_qml ALIAS muse::graphicaleffects_qml FOR muse_uicomponents)

qt_add_qml_module(muse_graphicaleffects_qml
    URI Muse.GraphicalEffects
    VERSION 1.0
    QML_FILES
        EffectDropShadow.qml
        EffectRectangularGlow.qml
        EffectFastBlur.qml
        EffectColorOverlay.qml
        RoundedCornersEffect.qml
)

qt_add_shaders(muse_graphicaleffects_qml muse_graphicaleffects_qml_shaders
    PREFIX
        "/qt/qml/Muse/GraphicalEffects"
    FILES
        "shaders/roundcorners.frag"
)
