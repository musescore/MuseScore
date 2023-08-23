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
#ifndef MU_NOTATION_EDITSTYLE_H
#define MU_NOTATION_EDITSTYLE_H

#include "ui_editstyle.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "inotationconfiguration.h"
#include "iinteractive.h"
#include "ui/iuiengine.h"
#include "engraving/iengravingfontsprovider.h"

#include "engraving/style/textstyle.h"

namespace mu::notation {
class EditStyle : public QDialog, private Ui::EditStyleBase
{
    Q_OBJECT

    INJECT(mu::context::IGlobalContext, globalContext)
    INJECT(mu::notation::INotationConfiguration, configuration)
    INJECT(mu::framework::IInteractive, interactive)
    INJECT(mu::ui::IUiEngine, uiEngine)
    INJECT(mu::engraving::IEngravingFontsProvider, engravingFonts)
    INJECT(mu::accessibility::IAccessibilityController, accessibilityController)

    Q_PROPERTY(QString currentPageCode READ currentPageCode WRITE setCurrentPageCode NOTIFY currentPageChanged)
    Q_PROPERTY(QString currentSubPageCode READ currentSubPageCode WRITE setCurrentSubPageCode NOTIFY currentSubPageChanged)

public:
    EditStyle(QWidget* = nullptr);
    EditStyle(const EditStyle&);

    QString currentPageCode() const;
    QString currentSubPageCode() const;

public slots:
    void accept();
    void reject();

    void setCurrentPageCode(const QString& code);
    void setCurrentSubPageCode(const QString& code);

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

    /// EditStylePage
    /// This is a type for a pointer to any QWidget that is a member of EditStyle.
    /// It's used to create static references to the pointers to pages.
    typedef QWidget* EditStyle::* EditStylePage;
    static EditStylePage pageForElement(EngravingItem*);

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
    void setStyleQVariantValue(StyleId id, const QVariant& value);
    void setStyleValue(StyleId id, const PropertyValue& value);

private slots:
    void selectChordDescriptionFile();
    void setChordStyle(bool);
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
    void on_resetTabStylesButton_clicked();
    void editUserStyleName();
    void endEditUserStyleName();
    void resetUserStyleName();

private:
    QString m_currentPageCode;
    QString m_currentSubPageCode;
};
}

Q_DECLARE_METATYPE(mu::notation::EditStyle)

#endif // MU_NOTATION_EDITSTYLE_H
