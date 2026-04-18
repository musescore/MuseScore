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

#pragma once

#include <functional>

#include "ui/view/widgetdialog.h"

#include "ui_editstyle.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "interactive/iinteractive.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuiengine.h"
#include "engraving/iengravingfontsprovider.h"

#include "engraving/style/textstyle.h"

#include "inotationsceneconfiguration.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QQuickView;
class QRadioButton;
class QWidget;

namespace mu::notation {
/*!
 * Full score and part style dialog: binds @c Ui::EditStyleBase widgets to @c engraving::StyleId values.
 *
 * The Notes page adds a programmatic Color @c QGroupBox (presets, schemes, swatches, apply-to, concert pitch,
 * reset) in @c classBegin(), then wraps the whole Notes tab (flags, color, notes,
 * alignment) in a @c QScrollArea so widget content scrolls like other native Style pages.
 */
class EditStyle : public muse::ui::WidgetDialog, private Ui::EditStyleBase
{
    Q_OBJECT

    Q_PROPERTY(QString currentPageCode READ currentPageCode WRITE setCurrentPageCode NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentSubPageCode READ currentSubPageCode WRITE setCurrentSubPageCode NOTIFY currentSubPageChanged)

    muse::GlobalInject<mu::notation::INotationSceneConfiguration> configuration;
    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<mu::engraving::IEngravingFontsProvider> engravingFonts;
    muse::ContextInject<mu::context::IGlobalContext> globalContext = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::ui::IUiEngine> uiEngine = { this };
    muse::ContextInject<muse::accessibility::IAccessibilityController> accessibilityController = { this };

public:
    EditStyle(QWidget* = nullptr);

    //! Wires style widgets, including the dynamic note-color UI on the Notes page.
    void classBegin() override;

    QString currentPageCode() const;
    QString currentSubPageCode() const;

public slots:
    void accept() override;
    void reject() override;

    void setCurrentPageCode(const QString& code);
    void setCurrentSubPageCode(const QString& code);
    void goToTextStylePage(const QString& code);
    void goToTextStylePage(int index);

signals:
    void currentPageChanged();
    void currentSubPageChanged();

private:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
    void changeEvent(QEvent*) override;
    void keyPressEvent(QKeyEvent* event) override;

    void retranslate();
    //! Retranslates labels and swatch captions in the note-color section after language change.
    void retranslateNoteColorSection();
    void setHeaderFooterMacroInfoText();
    void adjustPagesStackSize(int currentPageIndex);

    bool isBoolStyleRepresentedByButtonGroup(StyleId id);

    struct WidgetAndView {
        QWidget* widget = nullptr;      //!< Container placed in the page layout.
        QQuickView* view = nullptr;    //!< Quick scene for QML content, if used.
    };

    WidgetAndView createQmlWidget(QWidget* parent, const QUrl& source);

    /// EditStylePage
    /// This is a type for a pointer to any QWidget that is a member of EditStyle.
    /// It's used to create static references to the pointers to pages.
    typedef QWidget* EditStyle::* EditStylePage;

    struct StyleWidget {
        StyleId idx = StyleId::NOSTYLE;     //!< Style key written to the score.
        bool showPercent = false;           //!< Spin boxes show 0–100 when true.
        QObject* widget = nullptr;          //!< Bound control (combo, checkbox, color label, …).
        QToolButton* reset = nullptr;       //!< Resets @p idx to default when present.
    };

    QVector<StyleWidget> styleWidgets;
    const StyleWidget& styleWidget(StyleId id) const;

    class LineStyleSelect;
    std::vector<LineStyleSelect*> m_lineStyleSelects;

    std::vector<QComboBox*> verticalPlacementComboBoxes;

    QPushButton* buttonApplyToAllParts = nullptr;

    /**
     * @name Note color (Edit Style)
     * Widgets for the note-color preset, scheme, swatches, apply-to options, and concert pitch.
     **/
    ///@{
    QGroupBox* m_noteColorGroup = nullptr;                 //!< Container group box for all note-color widgets.
    QLabel* m_noteColorPresetLabel = nullptr;              //!< "Preset:" caption.
    QLabel* m_noteColorSchemeLabel = nullptr;              //!< "Coloring scheme:" caption.
    QComboBox* m_noteColorPresetCombo = nullptr;           //!< Default / Boomwhackers / Figurenotes / Custom.
    QComboBox* m_noteColorSchemeCombo = nullptr;           //!< @c NoteColoringScheme selector.
    QLabel* m_noteColorSwatchColorLabel = nullptr;         //!< Caption above the default color swatch.
    QWidget* m_noteColorSwatchesContainer = nullptr;       //!< Holds the 12 per-pitch color labels.
    QLabel* m_noteColorSchemeDescription = nullptr;        //!< Scheme-specific explanatory text.
    QGroupBox* m_noteColorApplyToGroupBox = nullptr;       //!< "Apply color to:" check-box group.
    QCheckBox* m_noteColorCbAccidental = nullptr;          //!< Bound to @c Sid::colorApplyToAccidental.
    QCheckBox* m_noteColorCbStem = nullptr;                //!< Bound to @c Sid::colorApplyToStem.
    QCheckBox* m_noteColorCbArticulation = nullptr;        //!< Bound to @c Sid::colorApplyToArticulation.
    QCheckBox* m_noteColorCbDot = nullptr;                 //!< Bound to @c Sid::colorApplyToDot.
    QCheckBox* m_noteColorCbBeam = nullptr;                //!< Bound to @c Sid::colorApplyToBeam.
    QGroupBox* m_noteColorPitchGroupBox = nullptr;         //!< Written vs concert pitch radio group.
    QRadioButton* m_noteColorRbWritten = nullptr;          //!< Color by written (displayed) pitch.
    QRadioButton* m_noteColorRbConcert = nullptr;          //!< Color by concert (sounding) pitch.
    QPushButton* m_noteColorResetBtn = nullptr;            //!< Reverts every note-color style to defaults.
    //! Rebuilds note-color controls from current style (queued after @c setValues()).
    std::function<void()> m_syncNoteColorUi;
    ///@}

    void unhandledType(const StyleWidget);
    PropertyValue getValue(StyleId idx);
    void setValues();

    const PropertyValue& styleValue(StyleId id) const;
    const PropertyValue& defaultStyleValue(StyleId id) const;
    bool hasDefaultStyleValue(StyleId id) const;
    bool dynamicsAndHairpinPosPropertiesHaveDefaultStyleValue() const;
    void setStyleQVariantValue(StyleId id, const QVariant& value);
    void setStyleValue(StyleId id, const PropertyValue& value);

private slots:
    // void selectChordDescriptionFile();
    // void setChordStyle(bool);
    void enableStyleWidget(const StyleId idx, bool enable);
    void enableVerticalSpreadClicked(bool);
    void disableVerticalSpreadClicked(bool);
    void toggleHeaderOddEven(bool);
    void toggleFooterOddEven(bool);
    void buttonClicked(QAbstractButton*);
    void setSwingParams(bool);
    void concertPitchToggled(bool);
    void lyricsDashMinLengthValueChanged(double);
    void lyricsDashMaxLengthValueChanged(double);
    void systemMinDistanceValueChanged(double);
    void systemMaxDistanceValueChanged(double);
    void resetStyleValue(int);
    void valueChanged(int);
    void textStyleChanged(int);
    void resetTextStyle(engraving::TextStylePropertyType type);
    void textStyleValueChanged(engraving::TextStylePropertyType type, const engraving::PropertyValue& value);
    void on_comboFBFont_currentIndexChanged(int index);
    void on_buttonTogglePagelist_clicked();
    void on_resetStylesButton_clicked();
    void on_pageRowSelectionChanged();
    void editUserStyleName();
    void endEditUserStyleName();
    void resetUserStyleName();
    void updateParenthesisIndicatingTiesGroupState();

private:
    QString m_currentPageCode;
    QString m_currentSubPageCode;
};
}
