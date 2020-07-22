//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

// this is a modified version of qt QSvgIconEngine

#ifndef __MICONENGINE_H__
#define __MICONENGINE_H__

#include <QIconEngine>
#include <QSharedData>
#include <QSettings>
#include <QVariant>
#include <QSvgRenderer>
#include <QPixmap>
#include <QPixmapCache>
#include <QFile>
#include <QPainter>
#include <QFileInfo>

class MIconEnginePrivate;

//---------------------------------------------------------
//   MIconEngine
//---------------------------------------------------------

class MIconEngine : public QIconEngine
{
public:
    static QString iconDirPath;

    MIconEngine();
    MIconEngine(const MIconEngine& other);
    ~MIconEngine();
    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state);
    QSize actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state);
    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state);

    void addPixmap(const QPixmap& pixmap, QIcon::Mode mode, QIcon::State state);
    void addFile(const QString& fileName, const QSize& size, QIcon::Mode mode, QIcon::State state);

    QString key() const;
    QIconEngine* clone() const;

private:
    QSharedDataPointer<MIconEnginePrivate> d;
};

#endif
