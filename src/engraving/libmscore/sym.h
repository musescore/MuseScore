/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef __SYM_H__
#define __SYM_H__

#include "symid.h"
#include "infrastructure/draw/geometry.h"

namespace Ms {
//---------------------------------------------------------
//   SmuflAnchorId
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

enum class SmuflAnchorId {
    stemDownNW,
    stemUpSE,
    stemDownSW,
    stemUpNW,
    cutOutNE,
    cutOutNW,
    cutOutSE,
    cutOutSW,
};

//---------------------------------------------------------
//   Sym
///   \cond PLUGIN_API \private \endcond
//---------------------------------------------------------

class Sym
{
public:
    Sym() = default;

    bool isValid() const;

    const std::vector<SymId>& subSymbols() const;
    void setSubSymbols(const std::vector<SymId>& subSymbols);

    int code() const;
    void setCode(int val);

    mu::RectF bbox() const;
    void setBbox(mu::RectF val);

    qreal advance() const;
    void setAdvance(qreal val);

    mu::PointF smuflAnchor(SmuflAnchorId anchorId);
    void setSmuflAnchor(SmuflAnchorId anchorId, const mu::PointF& newValue);

    static SymId name2id(const QString& s); // return noSym if not found
    static SymId oldName2id(const QString s);
    static const char* id2name(SymId id);

    static QString id2userName(SymId id);
    static SymId userName2id(const QString& userName);

    static const std::array<const char*, int(SymId::lastSym) + 1> symNames;
    static const std::array<const char*, int(SymId::lastSym) + 1> symUserNames;
    static const std::vector<SymId> commonScoreSymbols;

    static QHash<QString, SymId> nameToSymIdHash;

    struct OldName {
        const char* name;
        SymId symId;
    };
    static QHash<QString, SymId> oldNameToSymIdHash;
    static std::vector<OldName> oldNames;

private:
    int m_code = -1;
    mu::RectF m_bbox;
    qreal m_advance = 0.0;

    std::map<SmuflAnchorId, mu::PointF> m_smuflAnchors;
    std::vector<SymId> m_subSymbolIds; // not empty if this is a compound symbol
};
} // namespace Ms
#endif
