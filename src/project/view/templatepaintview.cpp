/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "templatepaintview.h"
#include "notation/imasternotation.h"
#include "log.h"

using namespace mu::project;
using namespace mu::notation;

TemplatePaintView::TemplatePaintView(QQuickItem* parent)
    : NotationPaintView(parent)
{
    setReadonly(true);
}

void TemplatePaintView::load(const QString& templatePath)
{
    if (templatePath.isEmpty()) {
        return;
    }

    if (m_templatePath == templatePath) {
        return;
    }

    m_templatePath = templatePath;

    NotationPaintView::load();
}

QString TemplatePaintView::zoomInSequence() const
{
    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut("zoomin");
    return QString::fromStdString(shortcut.sequence);
}

QString TemplatePaintView::zoomOutSequence() const
{
    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut("zoomout");
    return QString::fromStdString(shortcut.sequence);
}

void TemplatePaintView::adjustCanvas()
{
    qreal scaling = resolveDefaultScaling();

    if (qFuzzyIsNull(scaling) || scaling < 0) {
        return;
    }

    scale(scaling, QPoint());
    moveCanvasToCenter();
}

qreal TemplatePaintView::resolveDefaultScaling() const
{
    //! NOTE: this value was found experimentally
    constexpr qreal PROPORTION_FACTOR = 1.2;

    QRectF notationRect = notationContentRect();

    qreal widthScaling = width() * guiScaling() / notationRect.width() / PROPORTION_FACTOR;
    qreal heightScaling = height() * guiScaling() / notationRect.height() / PROPORTION_FACTOR;

    return std::min(widthScaling, heightScaling);
}

void TemplatePaintView::onViewSizeChanged()
{
    adjustCanvas();
}

void TemplatePaintView::onNotationSetup()
{
    INotationProjectPtr notationProject = notationCreator()->newProject();
    Ret ret = notationProject->load(m_templatePath);

    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    setNotation(notationProject->masterNotation()->notation());

    if (notationProject) {
        adjustCanvas();
    }
}
