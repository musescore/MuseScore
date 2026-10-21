/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "modularity/ioc.h"

#include "project/iprojectconfiguration.h"
#include "project/iprojectcreator.h"
#include "notation/notationtypes.h"
#include "context/iglobalcontext.h"
#include "notation/iinstrumentsrepository.h"

namespace mu::project {
class NewScoreModel : public QObject, public muse::Contextable
{
    Q_OBJECT

    QML_ELEMENT

    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<IProjectCreator> notationCreator;
    muse::GlobalInject<notation::IInstrumentsRepository> instrumentsRepository;
    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    explicit NewScoreModel(QObject* parent = nullptr);

    Q_INVOKABLE QString preferredScoreCreationMode() const;

    //! Languages that the names of the instruments in the new score can be shown in: a list of { code, name }.
    //! The first one (with an empty code) means the language of the interface. It is followed by the recently
    //! used languages, and then by the others. The language of the interface itself is not listed.
    Q_INVOKABLE QVariantList instrumentNamesLanguages() const;
    Q_INVOKABLE QString lastInstrumentNamesLanguage() const;

    //! An example of the names of an instrument as it would be added in the language, e.g. "Example: Clarinette in Si♭"
    Q_INVOKABLE QString instrumentNamesExample(const QString& languageCode) const;

    Q_INVOKABLE bool createScore(const QVariant& info);

private:
    project::ProjectCreateOptions parseOptions(const QVariantMap& info) const;
    notation::DurationType noteIconToDurationType(int noteIconCode) const;
    void updatePreferredScoreCreationMode(bool isScoreCreatedFromInstruments);
};
}
