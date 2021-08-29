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

namespace mu::notation {
class EditStyle : public QDialog, private Ui::EditStyleBase
{
    Q_OBJECT

    INJECT(notation, mu::context::IGlobalContext, globalContext)
    INJECT(notation, mu::notation::INotationConfiguration, configuration)
    INJECT(notation, mu::framework::IInteractive, interactive)
    INJECT(notation, mu::ui::IUiEngine, qmlEngineProvider)

public:
    EditStyle(QWidget* = nullptr);
    EditStyle(const EditStyle&);

    void setPage(int idx);
    void gotoElement(EngravingItem* e);
    static bool elementHasPage(EngravingItem* e);

    Q_INVOKABLE void goToChordTextSettings();

public slots:
    void accept();
    void reject();

private:
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);
    void changeEvent(QEvent*);

    void retranslate();
    void setHeaderFooterToolTip();
    void adjustPagesStackSize(int currentPageIndex);

    /// EditStylePage
    /// This is a type for a pointer to any QWidget that is a member of EditStyle.
    /// It's used to create static references to the pointers to pages.
    typedef QWidget* EditStyle::* EditStylePage;
    static EditStylePage pageForElement(EngravingItem*);

    struct StyleWidget {
        StyleId idx;
        bool showPercent;
        QObject* widget;
        QToolButton* reset;
    };

    QVector<StyleWidget> styleWidgets;
    const StyleWidget& styleWidget(StyleId id) const;

    std::vector<QComboBox*> lineStyleComboBoxes;
    std::vector<QComboBox*> verticalPlacementComboBoxes;
    std::vector<QComboBox*> horizontalPlacementComboBoxes;

    QPushButton* buttonApplyToAllParts = nullptr;

    void unhandledType(const StyleWidget);
    QVariant getValue(StyleId idx);
    void setValues();

    QVariant styleValue(StyleId id) const;
    QVariant defaultStyleValue(StyleId id) const;
    bool hasDefaultStyleValue(StyleId id) const;
    void setStyleValue(StyleId id, const QVariant& value);

    int numberOfPage;
    int pageListMap[50];

private slots:
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
    void resetTextStyle(Ms::Pid);
    void textStyleValueChanged(Ms::Pid, QVariant);
    void on_comboFBFont_currentIndexChanged(int index);
    void on_buttonTogglePagelist_clicked();
    void on_resetStylesButton_clicked();
    void editUserStyleName();
    void endEditUserStyleName();
    void resetUserStyleName();
    void pageListRowChanged(int);
    void pageListResetOrder();
    void pageListMoved(QModelIndex, int, int, QModelIndex, int);
    void stringToArray(std::string, int*);
    std::string arrayToString(int*);
    std::string ConsecutiveStr(int);
};
}

Q_DECLARE_METATYPE(mu::notation::EditStyle)

#endif // MU_NOTATION_EDITSTYLE_H
