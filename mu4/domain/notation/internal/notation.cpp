//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "notation.h"

#include <QPointF>
#include <QPainter>
#include <QFileInfo>

#include "log.h"

#include "config.h"
#include "io/filepath.h"

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/part.h"

#include "../notationerrors.h"
#include "notationinteraction.h"

#ifdef BUILD_UI_MU4
//! HACK Temporary hack to link libmscore
Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)

namespace Ms {
QString revision;
MasterSynthesizer* synti;
QString dataPath;
QString mscoreGlobalShare;
}
//! ---------
#endif

using namespace mu::domain::notation;
using namespace Ms;

Notation::Notation()
{
    m_scoreGlobal = new MScore(); //! TODO May be static?

    m_interaction = new NotationInteraction(this);

    m_interaction->noteAdded().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });

    m_interaction->dragChanged().onNotify(this, [this]() {
        notifyAboutNotationChanged();
    });
}

Notation::~Notation()
{
    delete m_score;
}

void Notation::init()
{
    MScore::init();         // initialize libmscore
}

mu::Ret Notation::load(const io::path& path)
{
    std::string syffix = io::syffix(path);

    //! NOTE For "mscz", "mscx" see MsczNotationReader
    //! for others see readers in importexport module
    auto reader = readers()->reader(syffix);
    if (!reader) {
        LOGE() << "not found reader for file: " << path;
        return make_ret(Ret::Code::InternalError);
    }

    return load(path, reader);
}

mu::Ret Notation::load(const io::path& path, const std::shared_ptr<INotationReader>& reader)
{
    if (m_score) {
        delete m_score;
        m_score = nullptr;
    }

    ScoreLoad sl;

    MasterScore* score = new MasterScore(m_scoreGlobal->baseStyle());
    Ret ret = doLoadScore(score, path, reader);
    if (ret) {
        m_score = score;
    }

    return ret;
}

mu::Ret Notation::doLoadScore(Ms::MasterScore* score,
                              const io::path& path,
                              const std::shared_ptr<INotationReader>& reader) const
{
    QFileInfo fi(io::pathToQString(path));
    score->setName(fi.completeBaseName());
    score->setImportedFilePath(fi.filePath());
    score->setMetaTag("originalFormat", fi.suffix().toLower());

    Ret ret = reader->read(score, path);
    if (!ret) {
        return ret;
    }

    score->connectTies();

    for (Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }
    score->rebuildMidiMapping();
    score->setSoloMute();
    for (Score* s : score->scoreList()) {
        s->setPlaylistDirty();
        s->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
        s->setLayoutAll();
    }
    score->updateChannel();
    //score->updateExpressive(MuseScore::synthesizer("Fluid"));
    score->setSaved(false);
    score->update();

    if (!score->sanityCheck(QString())) {
        return make_ret(Err::FileCorrupted);
    }

    return make_ret(Ret::Code::Ok);
}

mu::io::path Notation::path() const
{
    if (!m_score) {
        return io::path();
    }

    return io::pathFromQString(m_score->fileInfo()->canonicalFilePath());
}

void Notation::setViewSize(const QSizeF& vs)
{
    m_viewSize = vs;
}

void Notation::paint(QPainter* p, const QRect&)
{
    const QList<Ms::Page*>& mspages = m_score->pages();

    if (mspages.isEmpty()) {
        p->drawText(10, 10, "no pages");
        return;
    }

    Ms::Page* page = mspages.first();
    page->draw(p);

    p->fillRect(page->bbox(), QColor("#ffffff"));

    QList<Ms::Element*> ell = page->elements();
    for (const Ms::Element* e : ell) {
        if (!e->visible()) {
            continue;
        }

        e->itemDiscovered = false;
        QPointF pos(e->pagePos());
        //LOGI() << e->name() << ", x: " << pos.x() << ", y: " << pos.y() << "\n";

        p->translate(pos);

        e->draw(p);

        p->translate(-pos);
    }

    m_interaction->paint(p);
}

void Notation::notifyAboutNotationChanged()
{
    m_notationChanged.notify();
}

INotationInteraction* Notation::interaction() const
{
    return m_interaction;
}

mu::async::Notification Notation::notationChanged() const
{
    return m_notationChanged;
}

Ms::Score* Notation::score() const
{
    return m_score;
}

QSizeF Notation::viewSize() const
{
    return m_viewSize;
}
