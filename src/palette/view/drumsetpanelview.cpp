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
#include "drumsetpanelview.h"

#include "widgets/drumsetpalette.h"

using namespace mu::notation;

namespace mu::palette {
class DrumsetPaletteAdapter : public ui::IDisplayableWidget
{
public:
    DrumsetPaletteAdapter()
        : m_drumsetPaletteWidget(new DrumsetPalette())
    {
    }

    ~DrumsetPaletteAdapter() override
    {
        delete m_drumsetPaletteWidget;
    }

    void setNotation(INotationPtr notation)
    {
        m_drumsetPaletteWidget->setNotation(notation);
    }

    void updateDrumset()
    {
        m_drumsetPaletteWidget->updateDrumset();
    }

    async::Channel<QString> pitchNameChanged() const
    {
        return m_drumsetPaletteWidget->pitchNameChanged();
    }

private:
    QWidget* qWidget() override
    {
        return m_drumsetPaletteWidget;
    }

    bool handleEvent(QEvent* event) override
    {
        return m_drumsetPaletteWidget->handleEvent(event);
    }

    DrumsetPalette* m_drumsetPaletteWidget = nullptr;
};
}

using namespace mu::palette;

DrumsetPanelView::DrumsetPanelView(QQuickItem* parent)
    : WidgetView(parent)
{
}

QString DrumsetPanelView::pitchName() const
{
    return m_pitchName;
}

void DrumsetPanelView::editDrumset()
{
    dispatcher()->dispatch("edit-drumset");
}

void DrumsetPanelView::componentComplete()
{
    WidgetView::componentComplete();

    auto drumsetPalette = std::make_shared<DrumsetPaletteAdapter>();

    auto updateView = [this, drumsetPalette]() {
        drumsetPalette->updateDrumset();
        update();
    };

    auto initDrumsetPalette = [this, updateView, drumsetPalette]() {
        INotationPtr notation = globalContext()->currentNotation();
        drumsetPalette->setNotation(notation);
        updateView();

        if (!notation) {
            return;
        }

        notation->interaction()->noteInput()->stateChanged().onNotify(this, [updateView]() {
            updateView();
        });
    };

    globalContext()->currentNotationChanged().onNotify(this, [initDrumsetPalette]() {
        initDrumsetPalette();
    });

    drumsetPalette->pitchNameChanged().onReceive(this, [this](const QString& pitchName) {
        m_pitchName = pitchName;
        emit pitchNameChanged();
    });

    setWidget(drumsetPalette);

    initDrumsetPalette();
}
