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

#include "stringutils.h"

#include "notation/imasternotation.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace mu::actions;

TemplatePaintView::TemplatePaintView(QQuickItem* parent)
    : NotationPaintView(parent)
{
    setReadonly(true);

    m_notationProject = notationCreator()->newProject();
}

TemplatePaintView::~TemplatePaintView()
{
    resetNotation();
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
    return shortcutsTitleByActionCode("zoomin");
}

QString TemplatePaintView::zoomOutSequence() const
{
    return shortcutsTitleByActionCode("zoomout");
}

void TemplatePaintView::adjustCanvas()
{
    qreal scaling = resolveDefaultScaling();

    if (qFuzzyIsNull(scaling) || scaling < 0) {
        return;
    }

    setScaling(scaling, PointF());
    moveCanvasToCenter();
}

qreal TemplatePaintView::resolveDefaultScaling() const
{
    //! NOTE: this value was found experimentally
    constexpr qreal PROPORTION_FACTOR = 1.2;

    RectF notationRect = notationContentRect();

    qreal widthScaling = width() / notationRect.width() / PROPORTION_FACTOR;
    qreal heightScaling = height() / notationRect.height() / PROPORTION_FACTOR;

    return std::min(widthScaling, heightScaling);
}

void TemplatePaintView::onViewSizeChanged()
{
    adjustCanvas();
}

void TemplatePaintView::onNotationSetup()
{
    resetNotation();

    m_notationProject = notationCreator()->newProject();

    Ret ret = m_notationProject->load(m_templatePath);

    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    setNotation(m_notationProject->masterNotation()->notation());

    if (m_notationProject) {
        adjustCanvas();
    }
}

void TemplatePaintView::resetNotation()
{
    setNotation(nullptr);

    m_notationProject = nullptr;
}

QString TemplatePaintView::shortcutsTitleByActionCode(const ActionCode& code) const
{
    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(code);
    return shortcuts::sequencesToNativeText(shortcut.sequences);
}
