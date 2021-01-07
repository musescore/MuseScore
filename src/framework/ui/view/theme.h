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
#include "ui/iplatformtheme.h"
#include "async/asyncable.h"

namespace mu::framework {
class Theme : public QObject, public ITheme, public async::Asyncable
{
    Q_OBJECT

    INJECT(ui, IUiConfiguration, configuration)
    INJECT(ui, IPlatformTheme, platformTheme)

    Q_PROPERTY(QColor backgroundPrimaryColor READ backgroundPrimaryColor NOTIFY dataChanged)
    Q_PROPERTY(QColor backgroundSecondaryColor READ backgroundSecondaryColor NOTIFY dataChanged)
    Q_PROPERTY(QColor popupBackgroundColor READ popupBackgroundColor NOTIFY dataChanged)
    Q_PROPERTY(QColor textFieldColor READ textFieldColor NOTIFY dataChanged)
    Q_PROPERTY(QColor strokeColor READ strokeColor NOTIFY dataChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY dataChanged)
    Q_PROPERTY(QColor buttonColor READ buttonColor NOTIFY dataChanged)
    Q_PROPERTY(QColor fontPrimaryColor READ fontPrimaryColor NOTIFY dataChanged)
    Q_PROPERTY(QColor fontSecondaryColor READ fontSecondaryColor NOTIFY dataChanged)

    Q_PROPERTY(qreal accentOpacityNormal READ accentOpacityNormal NOTIFY dataChanged)
    Q_PROPERTY(qreal accentOpacityHit READ accentOpacityHit NOTIFY dataChanged)
    Q_PROPERTY(qreal accentOpacityHover READ accentOpacityHover NOTIFY dataChanged)

    Q_PROPERTY(qreal buttonOpacityNormal READ buttonOpacityNormal NOTIFY dataChanged)
    Q_PROPERTY(qreal buttonOpacityHover READ buttonOpacityHover NOTIFY dataChanged)
    Q_PROPERTY(qreal buttonOpacityHit READ buttonOpacityHit NOTIFY dataChanged)

    Q_PROPERTY(qreal itemOpacityDisabled READ itemOpacityDisabled NOTIFY dataChanged)

    Q_PROPERTY(QFont bodyFont READ bodyFont NOTIFY dataChanged)
    Q_PROPERTY(QFont bodyBoldFont READ bodyBoldFont NOTIFY dataChanged)
    Q_PROPERTY(QFont largeBodyFont READ largeBodyFont NOTIFY dataChanged)
    Q_PROPERTY(QFont largeBodyBoldFont READ largeBodyBoldFont NOTIFY dataChanged)
    Q_PROPERTY(QFont tabFont READ tabFont NOTIFY dataChanged)
    Q_PROPERTY(QFont tabBoldFont READ tabBoldFont NOTIFY dataChanged)
    Q_PROPERTY(QFont headerFont READ headerFont NOTIFY dataChanged)
    Q_PROPERTY(QFont headerBoldFont READ headerBoldFont NOTIFY dataChanged)
    Q_PROPERTY(QFont titleBoldFont READ titleBoldFont NOTIFY dataChanged)

    Q_PROPERTY(QFont iconsFont READ iconsFont NOTIFY dataChanged)
    Q_PROPERTY(QFont musicalFont READ musicalFont NOTIFY dataChanged)

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

    QFont bodyFont() const override;
    QFont bodyBoldFont() const override;
    QFont largeBodyFont() const override;
    QFont largeBodyBoldFont() const override;
    QFont tabFont() const override;
    QFont tabBoldFont() const override;
    QFont headerFont() const override;
    QFont headerBoldFont() const override;
    QFont titleBoldFont() const override;

    QFont iconsFont() const override;
    QFont musicalFont() const override;

    qreal accentOpacityNormal() const override;
    qreal accentOpacityHover() const override;
    qreal accentOpacityHit() const override;

    qreal buttonOpacityNormal() const override;
    qreal buttonOpacityHover() const override;
    qreal buttonOpacityHit() const override;

    qreal itemOpacityDisabled() const override;

    async::Notification themeChanged() const override;

signals:
    void dataChanged();

private:
    QHash<int, QVariant> currentThemeProperties() const;

    void initUiFonts();
    void initIconsFont();
    void initMusicalFont();

    void setupUiFonts();
    void setupIconsFont();
    void setupMusicFont();

    void setupWidgetTheme();

    void notifyAboutThemeChanged();

    QFont m_bodyFont;
    QFont m_bodyBoldFont;
    QFont m_largeBodyFont;
    QFont m_largeBodyBoldFont;
    QFont m_tabFont;
    QFont m_tabBoldFont;
    QFont m_headerFont;
    QFont m_headerBoldFont;
    QFont m_titleBoldFont;
    QFont m_iconsFont;
    QFont m_musicalFont;

    async::Notification m_themeChanged;
};
}

#endif // MU_FRAMEWORK_THEME_H
