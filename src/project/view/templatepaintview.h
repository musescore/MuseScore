/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PROJECT_TEMPLATEPAINTVIEW_H
#define MU_PROJECT_TEMPLATEPAINTVIEW_H

#include "modularity/ioc.h"
#include "project/iprojectcreator.h"
#include "notation/view/abstractnotationpaintview.h"
#include "shortcuts/ishortcutsregister.h"

namespace mu::project {
class TemplatePaintView : public notation::AbstractNotationPaintView
{
    Q_OBJECT

    INJECT(IProjectCreator, notationCreator)
    INJECT(muse::shortcuts::IShortcutsRegister, shortcutsRegister)

public:
    explicit TemplatePaintView(QQuickItem* parent = nullptr);
    ~TemplatePaintView() override;

    Q_INVOKABLE void load(const QString& templatePath);

    Q_INVOKABLE QString zoomInSequence() const;
    Q_INVOKABLE QString zoomOutSequence() const;

private slots:
    void onViewSizeChanged() override;

private:
    void onNotationSetup() override;

    void resetNotation();

    QString shortcutsTitleByActionCode(const muse::actions::ActionCode& code) const;

    void adjustCanvas();
    qreal resolveDefaultScaling() const;

    QString m_templatePath;

    INotationProjectPtr m_notationProject = nullptr;
};
}

#endif // MU_PROJECT_TEMPLATEPAINTVIEW_H
