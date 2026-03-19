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
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Use static library for consistency with other MuseScore thirdparty libs
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library" FORCE)
set(BUILD_PROGRAMS OFF CACHE BOOL "Build extra utilities" FORCE)

add_subdirectory(fdk-aac-2.0.3 EXCLUDE_FROM_ALL)

muse_create_thirdparty_module(fdk_aac INTERFACE fdk-aac)

target_no_warning(fdk-aac -Wno-unused-parameter)
target_no_warning(fdk-aac -Wno-unused-variable)
target_no_warning(fdk-aac -Wno-deprecated-declarations)
target_no_warning(fdk-aac -Wno-type-limits)
target_no_warning(fdk-aac -Wno-unknown-pragmas)
target_no_warning(fdk-aac -Wno-conversion)
target_no_warning(fdk-aac -Wno-uninitialized)
target_no_warning(fdk-aac -Wno-shift-negative-value)
target_no_warning(fdk-aac -Wno-implicit-fallthrough)
target_no_warning(fdk-aac -Wno-empty-body)
target_no_warning(fdk-aac -Wno-attributes)
target_no_warning(fdk-aac -Wc-no-absolute-value)
target_no_warning(fdk-aac -Wc-no-tautological-pointer-compare)
target_no_warning(fdk-aac -Wc-no-array-parameter)
target_no_warning(fdk-aac -Wno-unused-but-set-variable)
target_no_warning(fdk-aac -WMSVC-no-translation-unit-is-empty)
target_no_warning(fdk-aac -WMSVC-no-nonstandard-extension-used)
target_no_warning(fdk-aac -WMSVC-no-assignment-within-conditional-expression)
target_no_warning(fdk-aac -WMSVC-no-hides-previous)
target_no_warning(fdk-aac -WMSVC-no-undefined-assuming-extern)
target_no_warning(fdk-aac -WMSVC-named-type-definition-in-parentheses)
target_no_warning(fdk-aac -WMSVC-no-benign-redefinition)
target_no_warning(fdk-aac -WMSVC-no-unreachable)
target_no_warning(fdk-aac -WMSVC-no-cast-truncates-constant-value)
