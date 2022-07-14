/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "splashscreen.h"

#include <QPainter>
#include <QSvgRenderer>

#include "version.h"

using namespace mu::appshell;

static const QString imagePath(":/resources/LoadingScreen.svg");

static const QSize splashScreenSize(810, 405);

static const QColor messageColor("#99FFFFFF");
static const QRectF messageRect(splashScreenSize.width() / 2, 269, 0, 0);

static const QString website("www.musescore.org");
static const QRectF websiteRect(splashScreenSize.width() - 48, splashScreenSize.height() - 48, 0, 0);

static const QColor versionNumberColor("#22A0F4");
static constexpr qreal versionNumberSpacing = 5.0;

SplashScreen::SplashScreen()
    : QSplashScreen(QPixmap(splashScreenSize)),
    m_backgroundRenderer(new QSvgRenderer(imagePath, this))
{
    // Can't make translatable, because translation system not yet initialized
    showMessage("Loading…");
}

void SplashScreen::drawContents(QPainter* painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    // Draw background
    m_backgroundRenderer->render(painter);

    // Draw message
    // Can't use font from settings, because that's not yet initialized
    QFont font(QString::fromStdString(uiConfiguration()->defaultFontFamily()));
    font.setPixelSize(uiConfiguration()->defaultFontSize());

    painter->setFont(font);

    QPen pen(messageColor);
    painter->setPen(pen);

    painter->drawText(messageRect, Qt::AlignTop | Qt::AlignHCenter | Qt::TextDontClip, message());

    // Draw website URL
    QRectF websiteBoundingRect;
    painter->drawText(websiteRect, Qt::AlignBottom | Qt::AlignRight | Qt::TextDontClip, website, &websiteBoundingRect);

    // Draw version number
    pen.setColor(versionNumberColor);
    painter->setPen(pen);

    painter->drawText(websiteRect.translated(0.0, -websiteBoundingRect.height() - versionNumberSpacing),
                      Qt::AlignBottom | Qt::AlignRight | Qt::TextDontClip,
                      QString("Version %1").arg(QString::fromStdString(framework::Version::fullVersion())));
}
