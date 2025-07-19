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
#include "drumsetpanelview.h"

#include "widgets/drumsetpalette.h"

using namespace mu::notation;

namespace mu::palette {
class DrumsetPaletteAdapter : public muse::uicomponents::IDisplayableWidget
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

    muse::async::Channel<QString> pitchNameChanged() const
    {
        return m_drumsetPaletteWidget->pitchNameChanged();
    }

    DrumsetPalette* drumsetPalette() const
    {
        return m_drumsetPaletteWidget;
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

void DrumsetPanelView::customizeKit()
{
    dispatcher()->dispatch("customize-kit");
}

void DrumsetPanelView::componentComplete()
{
    WidgetView::componentComplete();

    m_adapter = std::make_shared<DrumsetPaletteAdapter>();

    initDrumsetPalette();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        initDrumsetPalette();
    });

    updateColors();
    notationConfiguration()->foregroundChanged().onNotify(this, [this]() {
        updateColors();
    });

    m_adapter->pitchNameChanged().onReceive(this, [this](const QString& pitchName) {
        setPitchName(pitchName);
    });

    setWidget(m_adapter);
}

void DrumsetPanelView::initDrumsetPalette()
{
    auto updateView = [this]() {
        m_adapter->updateDrumset();
        update();
    };

    INotationPtr notation = globalContext()->currentNotation();
    m_adapter->setNotation(notation);

    updateView();

    if (!notation) {
        return;
    }

    notation->interaction()->noteInput()->stateChanged().onNotify(this, [updateView]() {
        updateView();
    });
}

void DrumsetPanelView::updateColors()
{
    TRACEFUNC;

    const DrumsetPalette* palette = m_adapter->drumsetPalette();
    PaletteWidget* widget = palette->paletteWidget();

    PaletteWidget::PaintOptions options = widget->paintOptions();
    options.backgroundColor = notationConfiguration()->foregroundColor();
    options.selectionColor = engravingConfiguration()->selectionColor();

    if (engravingConfiguration()->scoreInversionEnabled()) {
        options.linesColor = engravingConfiguration()->scoreInversionColor();
    } else {
        options.linesColor = engravingConfiguration()->defaultColor();
    }

    options.useElementColors = true;
    options.colorsInverionsEnabled = true;

    widget->setPaintOptions(options);

    update();
}

void DrumsetPanelView::setPitchName(const QString& name)
{
    if (m_pitchName == name) {
        return;
    }

    m_pitchName = name;
    emit pitchNameChanged();
}
