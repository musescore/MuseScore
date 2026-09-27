/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <optional>

#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "context/iglobalcontext.h"
#include "project/iconvertfiletoscoreservice.h"

namespace mu::project {
class ReviewScoreQualityModel : public QObject, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(Step step READ step NOTIFY stepChanged)

    QML_ELEMENT

    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<IConvertFileToScoreService> convertFileToScoreService = { this };

public:
    enum class Step {
        Rating,
        Feedback
    };
    Q_ENUM(Step)

    explicit ReviewScoreQualityModel(QObject* parent = nullptr);

    Step step() const;

    Q_INVOKABLE void submitGood();
    Q_INVOKABLE void submitBad();
    Q_INVOKABLE void submitFeedback(const QString& comment);
    Q_INVOKABLE void skipFeedback();

signals:
    void stepChanged();
    void closeRequested();

private:
    std::optional<int> currentScoreId() const;
    void closeAndOpenNotationPage();

    Step m_step = Step::Rating;
    std::optional<ReviewRating> m_rating;
};
}
