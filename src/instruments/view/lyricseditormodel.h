#ifndef LYRICSEDITORMODEL_H
#define LYRICSEDITORMODEL_H

#include <QVariant>

#include "abstractinstrumentpaneltreeitem.h"
#include "modularity/ioc.h"
#include "notation/inotationparts.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "instrumentstypes.h"
#include "iselectinstrumentscenario.h"

namespace mu::instruments {
class LyricsEditorModel : public QObject, public actions::Actionable, public async::Asyncable
{
    Q_OBJECT

    INJECT(instruments, context::IGlobalContext, context)

public:
    LyricsEditorModel();

    Q_INVOKABLE QString updateLyrics();

signals:
    void lyricsUpdated();
};
}

#endif // LYRICSEDITORMODEL_H
