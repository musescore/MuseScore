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

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QWidget;

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "interactive/iinteractive.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuiengine.h"
#include "engraving/iengravingfontsprovider.h"

#include "engraving/style/textstyle.h"

#include "inotationsceneconfiguration.h"

class QQuickView;

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

    //! Page list identifier for the style sidebar (persisted view state).
    QString currentPageCode() const;
    //! Sub-page (e.g. text style) identifier within the current page.
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

    //! Refreshes all translatable strings from the UI file and note-color section.
    void retranslate();
    //! Retranslates labels and swatch captions in the note-color section after language change.
    void retranslateNoteColorSection();
    //! Sets help text for header/footer macro placeholders on the page style tab.
    void setHeaderFooterMacroInfoText();
    //! Resizes the stacked widget so the selected style page fits without excess blank space.
    void adjustPagesStackSize(int currentPageIndex);

    //! @c true when a style bool is edited via an exclusive @c QButtonGroup (not a plain checkbox).
    bool isBoolStyleRepresentedByButtonGroup(StyleId id);

    //! Host widget plus optional @c QQuickView for embedded QML style controls.
    struct WidgetAndView {
        QWidget* widget = nullptr;      //!< Container placed in the page layout.
        QQuickView* view = nullptr;    //!< Quick scene for QML content, if used.
    };

    //! Creates a @c QWidget embedding QML from @p source (e.g. rests/flags selectors).
    WidgetAndView createQmlWidget(QWidget* parent, const QUrl& source);

    /// EditStylePage
    /// This is a type for a pointer to any QWidget that is a member of EditStyle.
    /// It's used to create static references to the pointers to pages.
    typedef QWidget* EditStyle::* EditStylePage;

    //! One style control plus optional reset button; used by @c getValue() / @c setValues().
    struct StyleWidget {
        StyleId idx = StyleId::NOSTYLE;     //!< Style key written to the score.
        bool showPercent = false;           //!< Spin boxes show 0–100 when true.
        QObject* widget = nullptr;          //!< Bound control (combo, checkbox, color label, …).
        QToolButton* reset = nullptr;       //!< Resets @p idx to default when present.
    };

    //! All registered controls in declaration order (order matters for reset-all).
    QVector<StyleWidget> styleWidgets;
    //! Looks up the @c StyleWidget row for @p id (asserts if missing).
    const StyleWidget& styleWidget(StyleId id) const;

    class LineStyleSelect;
    //! Dash/gap spin boxes for line style customizations (ottava, pedal, …).
    std::vector<LineStyleSelect*> m_lineStyleSelects;

    //! Shared above/below items for line and text placement combos.
    std::vector<QComboBox*> verticalPlacementComboBoxes;

    //! Applies the current page of style changes to every part in the score.
    QPushButton* buttonApplyToAllParts = nullptr;

    /**
     * @name Note color (Edit Style)
     * Widgets for the note-color preset, scheme, swatches, apply-to options, and concert pitch.
     **/
    ///@{
    QGroupBox* m_noteColorGroup = nullptr;
    QLabel* m_noteColorPresetLabel = nullptr;
    QLabel* m_noteColorSchemeLabel = nullptr;
    QComboBox* m_noteColorPresetCombo = nullptr;
    QComboBox* m_noteColorSchemeCombo = nullptr;
    QLabel* m_noteColorSwatchColorLabel = nullptr;
    QWidget* m_noteColorSwatchesContainer = nullptr;
    QLabel* m_noteColorSchemeDescription = nullptr;
    QGroupBox* m_noteColorApplyToGroupBox = nullptr;
    QCheckBox* m_noteColorCbAccidental = nullptr;
    QCheckBox* m_noteColorCbStem = nullptr;
    QCheckBox* m_noteColorCbArticulation = nullptr;
    QCheckBox* m_noteColorCbDot = nullptr;
    QCheckBox* m_noteColorCbBeam = nullptr;
    QGroupBox* m_noteColorPitchGroupBox = nullptr;
    QRadioButton* m_noteColorRbWritten = nullptr;
    QRadioButton* m_noteColorRbConcert = nullptr;
    QPushButton* m_noteColorResetBtn = nullptr;
    //! Rebuilds note-color controls from current style (queued after @c setValues()).
    std::function<void()> m_syncNoteColorUi;
    ///@}

    //! Asserts on debug builds when a @c StyleWidget has no matching @c getValue / @c setValues handler.
    void unhandledType(const StyleWidget);
    //! Reads the current value from the widget registered for @p idx (spin box, combo, color label, etc.).
    PropertyValue getValue(StyleId idx);
    //! Loads the notation style into all @c styleWidgets controls (blocked signals while applying).
    void setValues();

    //! Live style value from the open notation (master or current part).
    const PropertyValue& styleValue(StyleId id) const;
    //! Factory default for @p id (for reset buttons and comparisons).
    const PropertyValue& defaultStyleValue(StyleId id) const;
    //! @c true when the score value for @p id equals the style default (@c defaultStyleValue(id) == @c styleValue(id)).
    bool hasDefaultStyleValue(StyleId id) const;
    //! Whether dynamics/hairpin placement combos should expose score-specific defaults.
    bool dynamicsAndHairpinPosPropertiesHaveDefaultStyleValue() const;
    //! Writes @p value into the notation style after converting from Qt types.
    void setStyleQVariantValue(StyleId id, const QVariant& value);
    //! Writes @p value into the notation style (undoable).
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
