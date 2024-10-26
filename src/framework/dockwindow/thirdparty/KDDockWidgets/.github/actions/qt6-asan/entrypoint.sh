#!/bin/sh -l

# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
cd /github/workspace/ || exit 1
env
cmake --preset=ci-dev-asan-qt6 -DCMAKE_PREFIX_PATH=/Qt6 && cd build-ci-dev-asan-qt6/ && ninja && ctest -j4 --output-on-failure
