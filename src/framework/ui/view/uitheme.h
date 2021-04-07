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

#ifndef MU_UI_UITHEME_H
#define MU_UI_UITHEME_H

#include <QObject>

#include "modularity/ioc.h"
#include "ui/iuiconfiguration.h"
#include "async/asyncable.h"

namespace mu::ui {
class UiTheme : public QObject, public async::Asyncable
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
    Q_PROPERTY(QColor linkColor READ linkColor NOTIFY themeChanged)
    Q_PROPERTY(QColor focusColor READ focusColor NOTIFY themeChanged)

    Q_PROPERTY(qreal accentOpacityNormal READ accentOpacityNormal NOTIFY themeChanged)
    Q_PROPERTY(qreal accentOpacityHit READ accentOpacityHit NOTIFY themeChanged)
    Q_PROPERTY(qreal accentOpacityHover READ accentOpacityHover NOTIFY themeChanged)

    Q_PROPERTY(qreal buttonOpacityNormal READ buttonOpacityNormal NOTIFY themeChanged)
    Q_PROPERTY(qreal buttonOpacityHover READ buttonOpacityHover NOTIFY themeChanged)
    Q_PROPERTY(qreal buttonOpacityHit READ buttonOpacityHit NOTIFY themeChanged)

    Q_PROPERTY(qreal itemOpacityDisabled READ itemOpacityDisabled NOTIFY themeChanged)

    Q_PROPERTY(QFont bodyFont READ bodyFont NOTIFY themeChanged)
    Q_PROPERTY(QFont bodyBoldFont READ bodyBoldFont NOTIFY themeChanged)
    Q_PROPERTY(QFont largeBodyFont READ largeBodyFont NOTIFY themeChanged)
    Q_PROPERTY(QFont largeBodyBoldFont READ largeBodyBoldFont NOTIFY themeChanged)
    Q_PROPERTY(QFont tabFont READ tabFont NOTIFY themeChanged)
    Q_PROPERTY(QFont tabBoldFont READ tabBoldFont NOTIFY themeChanged)
    Q_PROPERTY(QFont headerFont READ headerFont NOTIFY themeChanged)
    Q_PROPERTY(QFont headerBoldFont READ headerBoldFont NOTIFY themeChanged)
    Q_PROPERTY(QFont titleBoldFont READ titleBoldFont NOTIFY themeChanged)

    Q_PROPERTY(QFont iconsFont READ iconsFont NOTIFY themeChanged)
    Q_PROPERTY(QFont toolbarIconsFont READ toolbarIconsFont NOTIFY themeChanged)

    Q_PROPERTY(QFont musicalFont READ musicalFont NOTIFY themeChanged)

public:
    UiTheme(QObject* parent = nullptr);

    void init();
    void update();

    QColor backgroundPrimaryColor() const;
    QColor backgroundSecondaryColor() const;
    QColor popupBackgroundColor() const;
    QColor textFieldColor() const;
    QColor accentColor() const;
    QColor strokeColor() const;
    QColor buttonColor() const;
    QColor fontPrimaryColor() const;
    QColor fontSecondaryColor() const;
    QColor linkColor() const;
    QColor focusColor() const;

    QFont bodyFont() const;
    QFont bodyBoldFont() const;
    QFont largeBodyFont() const;
    QFont largeBodyBoldFont() const;
    QFont tabFont() const;
    QFont tabBoldFont() const;
    QFont headerFont() const;
    QFont headerBoldFont() const;
    QFont titleBoldFont() const;

    QFont iconsFont() const;
    QFont toolbarIconsFont() const;
    QFont musicalFont() const;

    qreal accentOpacityNormal() const;
    qreal accentOpacityHover() const;
    qreal accentOpacityHit() const;

    qreal buttonOpacityNormal() const;
    qreal buttonOpacityHover() const;
    qreal buttonOpacityHit() const;

    qreal itemOpacityDisabled() const;

signals:
    void themeChanged();

private:
    QColor colorByKey(ThemeStyleKey key) const;
    qreal realByKey(ThemeStyleKey key) const;
    const ThemeInfo& currentTheme() const;

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
    QFont m_toolbarIconsFont;
    QFont m_musicalFont;
};
}

#endif // MU_UI_UITHEME_H
