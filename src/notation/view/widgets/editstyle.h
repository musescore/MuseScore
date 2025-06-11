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
#ifndef MU_NOTATION_EDITSTYLE_H
#define MU_NOTATION_EDITSTYLE_H

#include "ui_editstyle.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "inotationconfiguration.h"
#include "iinteractive.h"
#include "ui/iuiconfiguration.h"
#include "ui/iuiengine.h"
#include "engraving/iengravingfontsprovider.h"

#include "engraving/style/textstyle.h"

class QQuickView;

namespace mu::notation {
class EditStyle : public QDialog, private Ui::EditStyleBase, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString currentPageCode READ currentPageCode WRITE setCurrentPageCode NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentSubPageCode READ currentSubPageCode WRITE setCurrentSubPageCode NOTIFY currentSubPageChanged)

    muse::Inject<mu::context::IGlobalContext> globalContext = { this };
    muse::Inject<mu::notation::INotationConfiguration> configuration = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };
    muse::Inject<muse::ui::IUiEngine> uiEngine = { this };
    muse::Inject<mu::engraving::IEngravingFontsProvider> engravingFonts = { this };
    muse::Inject<muse::accessibility::IAccessibilityController> accessibilityController = { this };

public:
    EditStyle(QWidget* = nullptr);

    QString currentPageCode() const;
    QString currentSubPageCode() const;

    static QString pageCodeForElement(const EngravingItem*);
    static QString subPageCodeForElement(const EngravingItem*);

public slots:
    void accept();
    void reject();

    void setCurrentPageCode(const QString& code);
    void setCurrentSubPageCode(const QString& code);
    void goToTextStylePage(const QString& code);

signals:
    void currentPageChanged();
    void currentSubPageChanged();

private:
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    void changeEvent(QEvent*);
    void keyPressEvent(QKeyEvent* event);

    void retranslate();
    void setHeaderFooterToolTip();
    void adjustPagesStackSize(int currentPageIndex);

    bool isBoolStyleRepresentedByButtonGroup(StyleId id);

    struct WidgetAndView {
        QWidget* widget = nullptr;
        QQuickView* view = nullptr;
    };

    WidgetAndView createQmlWidget(QWidget* parent, const QUrl& source);

    /// EditStylePage
    /// This is a type for a pointer to any QWidget that is a member of EditStyle.
    /// It's used to create static references to the pointers to pages.
    typedef QWidget* EditStyle::* EditStylePage;

    struct StyleWidget {
        StyleId idx = StyleId::NOSTYLE;
        bool showPercent = false;
        QObject* widget = nullptr;
        QToolButton* reset = nullptr;
    };

    QVector<StyleWidget> styleWidgets;
    const StyleWidget& styleWidget(StyleId id) const;

    class LineStyleSelect;
    std::vector<LineStyleSelect*> m_lineStyleSelects;

    std::vector<QComboBox*> verticalPlacementComboBoxes;
    std::vector<QComboBox*> horizontalPlacementComboBoxes;

    QPushButton* buttonApplyToAllParts = nullptr;

    void unhandledType(const StyleWidget);
    PropertyValue getValue(StyleId idx);
    void setValues();

    PropertyValue styleValue(StyleId id) const;
    PropertyValue defaultStyleValue(StyleId id) const;
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
    void textStyleValueChanged(engraving::TextStylePropertyType type, QVariant);
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

#endif // MU_NOTATION_EDITSTYLE_H
