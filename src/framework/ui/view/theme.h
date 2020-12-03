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

#ifndef MU_FRAMEWORK_THEME_H
#define MU_FRAMEWORK_THEME_H

#include <QObject>

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "ui/itheme.h"
#include "async/asyncable.h"

namespace mu::framework {
class Theme : public QObject, public ITheme, public async::Asyncable
{
    Q_OBJECT

    INJECT(ui, IUiConfiguration, configuration)

    Q_PROPERTY(QColor backgroundPrimaryColor READ backgroundPrimaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor backgroundSecondaryColor READ backgroundSecondaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor popupBackgroundColor READ popupBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textFieldColor READ textFieldColor NOTIFY themeChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor buttonColor READ buttonColor NOTIFY themeChanged)
    Q_PROPERTY(QColor fontPrimaryColor READ fontPrimaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor fontSecondaryColor READ fontSecondaryColor NOTIFY themeChanged)

    Q_PROPERTY(qreal accentOpacityNormal READ accentOpacityNormal NOTIFY themeChanged)
    Q_PROPERTY(qreal accentOpacityHit READ accentOpacityHit NOTIFY themeChanged)
    Q_PROPERTY(qreal accentOpacityHover READ accentOpacityHover NOTIFY themeChanged)

    Q_PROPERTY(qreal buttonOpacityNormal READ buttonOpacityNormal NOTIFY themeChanged)
    Q_PROPERTY(qreal buttonOpacityHover READ buttonOpacityHover NOTIFY themeChanged)
    Q_PROPERTY(qreal buttonOpacityHit READ buttonOpacityHit NOTIFY themeChanged)

    Q_PROPERTY(qreal itemOpacityDisabled READ itemOpacityDisabled NOTIFY themeChanged)

    Q_PROPERTY(QFont font READ font NOTIFY themeChanged)
    Q_PROPERTY(QFont musicalFont READ musicalFont NOTIFY themeChanged)

public:
    Theme(QObject* parent = nullptr);

    void init();
    void update();

    QColor backgroundPrimaryColor() const override;
    QColor backgroundSecondaryColor() const override;
    QColor popupBackgroundColor() const override;
    QColor textFieldColor() const override;
    QColor accentColor() const override;
    QColor strokeColor() const override;
    QColor buttonColor() const override;
    QColor fontPrimaryColor() const override;
    QColor fontSecondaryColor() const override;
    QFont font() const override;
    QFont musicalFont() const override;

    qreal accentOpacityNormal() const override;
    qreal accentOpacityHover() const override;
    qreal accentOpacityHit() const override;

    qreal buttonOpacityNormal() const override;
    qreal buttonOpacityHover() const override;
    qreal buttonOpacityHit() const override;

    qreal itemOpacityDisabled() const override;

signals:
    void themeChanged() override;

private:
    QHash<int, QVariant> currentThemeProperites() const;

    void initFont();
    void initMusicalFont();

    void setupWidgetTheme();

    QFont m_font;
    QFont m_musicalFont;
};
}

#endif // MU_FRAMEWORK_THEME_H
