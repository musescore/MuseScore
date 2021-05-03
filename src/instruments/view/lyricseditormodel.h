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

    Q_PROPERTY(bool mode READ mode WRITE setMode)
    Q_PROPERTY(bool expandRepeats READ expandRepeats WRITE setExpandRepeats)

public:
    LyricsEditorModel();

    Q_INVOKABLE QString updateLyrics();

    bool mode() const { return m_mode; }
    void setMode(bool b) { m_mode = b; }

    bool expandRepeats() const { return m_expandRepeats; }
    void setExpandRepeats(bool b) { m_expandRepeats = b; }

signals:
    void lyricsUpdated();

private:
    bool m_mode { true };
    bool m_expandRepeats { true };
};
}

#endif // LYRICSEDITORMODEL_H
