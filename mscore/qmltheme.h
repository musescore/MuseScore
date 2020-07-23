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

#ifndef MSF_QMLTHEME_H
#define MSF_QMLTHEME_H

#include <QObject>
#include <QPalette>
#include <QColor>

namespace msf {
class QmlTheme : public QObject
{
    Q_OBJECT

#define COLOR_PROPERTY(name, role) \
    Q_PROPERTY(QColor name READ get##name NOTIFY themeChanged) \
    QColor get##name() const { return _palette.color(role); \
    }

    COLOR_PROPERTY(window, QPalette::Window)
    COLOR_PROPERTY(windowText, QPalette::WindowText)
    COLOR_PROPERTY(base, QPalette::Base)
    COLOR_PROPERTY(alternateBase, QPalette::AlternateBase)
    COLOR_PROPERTY(text, QPalette::Text)
    COLOR_PROPERTY(button, QPalette::Button)
    COLOR_PROPERTY(buttonText, QPalette::ButtonText)
    COLOR_PROPERTY(brightText, QPalette::BrightText)
    COLOR_PROPERTY(toolTipBase, QPalette::ToolTipBase)
    COLOR_PROPERTY(toolTipText, QPalette::ToolTipText)
    COLOR_PROPERTY(link, QPalette::Link)
    COLOR_PROPERTY(linkVisited, QPalette::LinkVisited)
    COLOR_PROPERTY(highlight, QPalette::Highlight)
    COLOR_PROPERTY(highlightedText, QPalette::HighlightedText)

    COLOR_PROPERTY(shadow, QPalette::Shadow)

#undef COLOR_PROPERTY

public:
    QmlTheme(const QPalette& pal, QObject* parent = nullptr);

    void update(const QPalette& pal);

signals:
    void themeChanged();

private:

    QPalette _palette;
};
}

#endif // MSF_QMLTHEME_H
