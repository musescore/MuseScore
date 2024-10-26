#
# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sérgio Martins <sergio.martins@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.

# Dockerfile for qt6-asan GitHub Action

FROM iamsergio/kddw-qt6-asan
MAINTAINER Sergio Martins (sergio.martins@kdab.com)

ENV PATH=$PATH:/Qt6/bin/
ENV LD_LIBRARY_PATH=/Qt6/lib/
ENV QT_QUICK_BACKEND=software
ENV LSAN_OPTIONS=detect_leaks=0

COPY entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
