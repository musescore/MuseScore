/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "playbackproxymodel.h"

using namespace mu::inspector;

PlaybackProxyModel::PlaybackProxyModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent)
{
    setNotePlaybackModel(new NotePlaybackModel(this, repository));
    setArpeggioPlaybackModel(new ArpeggioPlaybackModel(this, repository));
    setFermataPlaybackModel(new FermataPlaybackModel(this, repository));
    setBreathPlaybackModel(new BreathPlaybackModel(this, repository));
    setGlissandoPlaybackModel(new GlissandoPlaybackModel(this, repository));
    setDynamicPlaybackModel(new DynamicPlaybackModel(this, repository));
    setHairpinPlaybackModel(new HairpinPlaybackModel(this, repository));
}

void PlaybackProxyModel::requestResetToDefaults()
{
    m_notePlaybackModel->requestResetToDefaults();
    m_arpeggioPlaybackModel->requestResetToDefaults();
    m_fermataPlaybackModel->requestResetToDefaults();
    m_breathPlaybackModel->requestResetToDefaults();
    m_glissandoPlaybackModel->requestResetToDefaults();
    m_dynamicPlaybackModel->requestResetToDefaults();
    m_hairpinPlaybackModel->requestResetToDefaults();
}

bool PlaybackProxyModel::hasAcceptableElements() const
{
    return m_notePlaybackModel->hasAcceptableElements()
           || m_arpeggioPlaybackModel->hasAcceptableElements()
           || m_fermataPlaybackModel->hasAcceptableElements()
           || m_breathPlaybackModel->hasAcceptableElements()
           || m_glissandoPlaybackModel->hasAcceptableElements()
           || m_dynamicPlaybackModel->hasAcceptableElements()
           || m_hairpinPlaybackModel->hasAcceptableElements();
}

QObject* PlaybackProxyModel::notePlaybackModel() const
{
    return m_notePlaybackModel;
}

QObject* PlaybackProxyModel::fermataPlaybackModel() const
{
    return m_fermataPlaybackModel;
}

QObject* PlaybackProxyModel::breathPlaybackModel() const
{
    return m_breathPlaybackModel;
}

QObject* PlaybackProxyModel::glissandoPlaybackModel() const
{
    return m_glissandoPlaybackModel;
}

QObject* PlaybackProxyModel::dynamicPlaybackModel() const
{
    return m_dynamicPlaybackModel;
}

QObject* PlaybackProxyModel::hairpinPlaybackModel() const
{
    return m_hairpinPlaybackModel;
}

QObject* PlaybackProxyModel::arpeggioPlaybackModel() const
{
    return m_arpeggioPlaybackModel;
}

void PlaybackProxyModel::setNotePlaybackModel(NotePlaybackModel* notePlaybackModel)
{
    m_notePlaybackModel = notePlaybackModel;
    emit notePlaybackModelChanged(m_notePlaybackModel);
}

void PlaybackProxyModel::setFermataPlaybackModel(FermataPlaybackModel* fermataPlaybackModel)
{
    m_fermataPlaybackModel = fermataPlaybackModel;
    emit fermataPlaybackModelChanged(m_fermataPlaybackModel);
}

void PlaybackProxyModel::setBreathPlaybackModel(BreathPlaybackModel* breathPlaybackModel)
{
    m_breathPlaybackModel = breathPlaybackModel;
    emit breathPlaybackModelChanged(m_breathPlaybackModel);
}

void PlaybackProxyModel::setGlissandoPlaybackModel(GlissandoPlaybackModel* glissandoPlaybackModel)
{
    m_glissandoPlaybackModel = glissandoPlaybackModel;
    emit glissandoPlaybackModelChanged(m_glissandoPlaybackModel);
}

void PlaybackProxyModel::setDynamicPlaybackModel(DynamicPlaybackModel* dynamicPlaybackModel)
{
    m_dynamicPlaybackModel = dynamicPlaybackModel;
    emit dynamicPlaybackModelChanged(m_dynamicPlaybackModel);
}

void PlaybackProxyModel::setHairpinPlaybackModel(HairpinPlaybackModel* hairpinPlaybackModel)
{
    m_hairpinPlaybackModel = hairpinPlaybackModel;
    emit hairpinPlaybackModelChanged(m_hairpinPlaybackModel);
}

void PlaybackProxyModel::setArpeggioPlaybackModel(ArpeggioPlaybackModel* arpeggioPlaybackModel)
{
    m_arpeggioPlaybackModel = arpeggioPlaybackModel;
    emit arpeggioPlaybackModelChanged(m_arpeggioPlaybackModel);
}
