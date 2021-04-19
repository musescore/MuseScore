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
#ifndef MU_USERSCORES_TEMPLATEPAINTVIEW_H
#define MU_USERSCORES_TEMPLATEPAINTVIEW_H

#include "modularity/ioc.h"
#include "notation/inotationcreator.h"
#include "notation/view/notationpaintview.h"
#include "shortcuts/ishortcutsregister.h"

namespace mu::userscores {
class TemplatePaintView : public notation::NotationPaintView
{
    Q_OBJECT

    INJECT(userscores, notation::INotationCreator, notationCreator)
    INJECT(userscores, shortcuts::IShortcutsRegister, shortcutsRegister)

public:
    explicit TemplatePaintView(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load(const QString& templatePath);

    Q_INVOKABLE QString zoomInSequence() const;
    Q_INVOKABLE QString zoomOutSequence() const;

private slots:
    void onViewSizeChanged() override;

private:
    void load() override;
    void adjustCanvas();
    qreal resolveDefaultScaling() const;

    QString m_templatePath;
};
}

#endif // MU_USERSCORES_TEMPLATEPAINTVIEW_H
