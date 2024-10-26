#
# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sérgio Martins <sergio.martins@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.

# The purpose of this Dockerfile is just for quickly getting a running python build for testing.

# Instructions:

# $ docker build -t kddw-pyside2 .
# $ docker run -it -v ~/Qt/5.15.2/gcc_64/:/Qt/ -v /tmp/.X11-unix:/tmp/.X11-unix kddw-pyside2
  # git checkout 2.0
  # cmake --preset=python -G Ninja . && cd build-python/ && ninja

  # Test with:
  # cd ../python/examples && rcc -g python -o rc_assets.py ../../examples/dockwidgets/resources_example.qrc
  # python3 main.py

FROM ubuntu:22.04
MAINTAINER Sergio Martins (sergio.martins@kdab.com)

ENV TZ=Europe/Berlin
ENV LC_CTYPE=C.UTF-8
ENV DISPLAY=:0
ENV PATH=$PATH:/Qt/bin/
ENV LD_LIBRARY_PATH=/Qt/lib/
ENV PYTHONPATH=/KDDockWidgets/build-python/python/

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update
RUN apt install build-essential software-properties-common git ninja-build libssl-dev mesa-common-dev libglu1-mesa-dev libglib2.0-dev libfontconfig libxkbcommon-x11-0 libdbus-1-3 cmake pip libxslt1-dev llvm-dev clang libxcb-keysyms1 libxcb-image0 libxcb-icccm4 libxcb-xinerama0  libxcb-shape0 libxcb-render-util0 -y
RUN pip3 install --index-url=http://download.qt.io/official_releases/QtForPython/ --trusted-host download.qt.io shiboken2 pyside2 shiboken2_generator

WORKDIR /
RUN git clone https://github.com/KDAB/KDDockWidgets.git
WORKDIR /KDDockWidgets/
