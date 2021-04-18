//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
