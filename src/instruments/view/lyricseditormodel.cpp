#include "qclipboard.h"
#include "lyricseditormodel.h"

mu::instruments::LyricsEditorModel::LyricsEditorModel()
{
    context()->currentNotationChanged().onNotify(this, [this]() {
        auto notation = context()->currentNotation();
        auto interaction = notation->interaction();
        interaction->textEditingChanged().onNotify(this, [this]() {
            updateLyrics();
            emit lyricsUpdated();
        });
        updateLyrics();
    });
}

QString mu::instruments::LyricsEditorModel::updateLyrics()
{
    auto notation = context()->currentNotation();
    auto interaction = notation->interaction();
    interaction->copyLyrics(m_mode, m_expandRepeats);
    return QApplication::clipboard()->text();
}
