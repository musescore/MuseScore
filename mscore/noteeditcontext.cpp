
#include "globals.h"
#include "noteeditcontext.h"
#include "preferences.h"
#include "shortcut.h"
#include "musescore.h"
#include "libmscore/element.h"
#include "libmscore/mscore.h"

namespace Ms {

NoteEditContext::NoteEditContext(QWidget *parent)
    : QWidget(parent)
    {

    entryContextToolbar = new QToolBar();
    entryContextToolbar->setObjectName("entry-tools");

    QToolBar* firstRowBar = new QToolBar();
    QToolBar* secondRowBar = new QToolBar();
    QToolBar* thirdRowBar = new QToolBar();

    QWidget* contxtToolbarWdg = new QWidget;
    QVBoxLayout *contxtToolbarLayout = new QVBoxLayout;


	static const char* row1[] = {
                       "note-input",  "pad-rest", "", "pad-dot", "pad-dotdot", "",
                       "sharp2", "sharp", "nat", "flat", "flat2"
                       };

    static const char* row2[] = {
                       "pad-note-128", "pad-note-64", "pad-note-32", "pad-note-16",
                       "pad-note-8", "pad-note-4", "pad-note-2", "pad-note-1", "note-breve", "note-longa",
                       };


    static const char* row3[] = {
                       "repitch", "tie", "flip", ""
                       };

    for (auto s : row1) {
          if (!*s)
                        firstRowBar->addSeparator();
                  else
                        firstRowBar->addAction(getAction(s));
                  }

    for (auto s : row2) {
                  if (!*s)
                        secondRowBar->addSeparator();
                  else
                        secondRowBar->addAction(getAction(s));
                  }

    for (auto s : row3) {
          if (!*s)
                        thirdRowBar->addSeparator();
          else
                thirdRowBar->addAction(getAction(s));
                  }

    contxtToolbarLayout->addWidget(firstRowBar);
    contxtToolbarLayout->addWidget(secondRowBar);
    contxtToolbarLayout->addWidget(thirdRowBar);
    contxtToolbarWdg->setLayout(contxtToolbarLayout);

    static const char* vbsh { "QToolButton:checked, QToolButton:pressed { color: white;}" };

    //QVector<QAction*> actionArray;

    for (int i = 0; i < VOICES; ++i) {
          QToolButton* tb = new QToolButton(this);
          if (preferences.globalStyle() == MuseScoreStyleType::LIGHT_FUSION)
                tb->setStyleSheet(vbsh);
          tb->setToolButtonStyle(Qt::ToolButtonTextOnly);
          QPalette p(tb->palette());
          p.setColor(QPalette::Base, MScore::selectColor[i]);
          tb->setPalette(p);
          QAction* a = getAction(voiceActions[i]);
          a->setCheckable(true);
          actionArray.append(a);
          tb->setDefaultAction(a);
          tb->setFocusPolicy(Qt::ClickFocus);
          thirdRowBar->addWidget(tb);
          }

    entryContextToolbar->addWidget(contxtToolbarWdg);
    entryContextMenu = new QMenu(this);

    QWidgetAction* noteAction = new QWidgetAction(entryContextMenu);
    noteAction->setDefaultWidget(entryContextToolbar);
    entryContextMenu->addAction(noteAction);

    for (int i = 0; i < actionArray.count(); i++)
          connect(actionArray[i], SIGNAL(triggered()),  entryContextMenu, SLOT(close()));

    connect(firstRowBar, SIGNAL(actionTriggered(QAction*)), entryContextMenu, SLOT(close()));
    connect(secondRowBar, SIGNAL(actionTriggered(QAction*)), entryContextMenu, SLOT(close()));
    connect(thirdRowBar, SIGNAL(actionTriggered(QAction*)), entryContextMenu, SLOT(close()));
    }

    NoteEditContext::~NoteEditContext(){

    }

    void NoteEditContext::open(const QPoint& pos)
    {
       entryContextMenu->exec(pos);
    }

}
