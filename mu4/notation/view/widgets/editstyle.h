//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#ifndef MU_NOTATION_EDITSTYLE_H
#define MU_NOTATION_EDITSTYLE_H

#include "ui_editstyle.h"
#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "inotationconfiguration.h"
#include "iinteractive.h"

namespace mu {
namespace notation {
class Score;
class EditStyle;

//---------------------------------------------------------
//   StyleWidget
//---------------------------------------------------------

struct StyleWidget {
    StyleId idx;
    bool showPercent;
    QObject* widget;
    QToolButton* reset;
};

//---------------------------------------------------------
//   EditStylePage
///   This is a type for a pointer to any QWidget that is a member of EditStyle.
///   It's used to create static references to the pointers to pages.
//---------------------------------------------------------

typedef QWidget* EditStyle::* EditStylePage;

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

class EditStyle : public QDialog, private Ui::EditStyleBase
{
    Q_OBJECT

    INJECT(notation, mu::context::IGlobalContext, globalContext)
    INJECT(notation, mu::notation::INotationConfiguration, configuration)
    INJECT(notation, mu::framework::IInteractive, interactive)

    QPushButton* buttonApplyToAllParts = nullptr;
    QVector<StyleWidget> styleWidgets;
    bool isTooBig = false;
    bool hasShown = false;

    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);
    QVariant getValue(StyleId idx);
    void setValues();

    QVariant styleValue(StyleId id) const;
    QVariant defaultStyleValue(StyleId id) const;
    bool hasDefaultStyleValue(StyleId id) const;
    void setStyleValue(StyleId id, const QVariant& value);

    const StyleWidget& styleWidget(StyleId id) const;

    static const std::map<ElementType, EditStylePage> PAGES;

private slots:
    void selectChordDescriptionFile();
    void setChordStyle(bool);
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
    void editUserStyleName();
    void endEditUserStyleName();
    void resetUserStyleName();

public:
    EditStyle(QWidget* = nullptr);
    EditStyle(const EditStyle&);

    static int metaTypeId();
};
}
}

Q_DECLARE_METATYPE(mu::notation::EditStyle)

#endif // MU_NOTATION_EDITSTYLE_H
