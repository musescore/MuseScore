#ifndef __NOTEEDITCONTEXT_H__
#define __NOTEEDITCONTEXT_H__

#include "globals.h"
#include "libmscore/element.h"
#include "libmscore/durationtype.h"
#include "libmscore/mscore.h"
#include "libmscore/mscoreview.h"
#include "libmscore/pos.h"

namespace Ms {

    class NoteEditContext : public QWidget {
        Q_OBJECT

        QToolBar* entryContextToolbar;
        QToolBar* firstRowBar;
        QToolBar* secondRowBar;
        QToolBar* thirdRowBar;

        QMenu* entryContextMenu;

        QWidget* contextToolbarWdg;
        QVBoxLayout* contextToolbarLayout;

        QVector<QAction*> actionArray;

    public:
        NoteEditContext(QWidget* parent = 0);
        ~NoteEditContext();
        void open(const QPoint& pos);

    };
}

#endif // NOTEEDITCONTEXT_H
