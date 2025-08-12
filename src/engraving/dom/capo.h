/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#ifndef MU_ENGRAVING_CAPO_H
#define MU_ENGRAVING_CAPO_H

#include "stafftextbase.h"

namespace mu::engraving {
class Capo final : public StaffTextBase
{
    OBJECT_ALLOCATOR(engraving, Capo)
    DECLARE_CLASSOF(ElementType::CAPO)

public:
    Capo(Segment* parent = nullptr, TextStyleType textStyleType = TextStyleType::STAFF);

    Capo* clone() const override;

    PropertyValue getProperty(Pid id) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& val) override;
    void setXmlText(const String& text) override;

    bool isEditable() const override;

    const CapoParams& params() const;
    void setParams(const CapoParams& params);

    bool shouldAutomaticallyGenerateText() const;
    String generateText(size_t stringCount) const;

private:
    CapoParams m_params;
    bool m_shouldAutomaticallyGenerateText = true;
    String m_customText;
};

class AbstractCapoTransposeState : public std::enable_shared_from_this<AbstractCapoTransposeState>
{
public:
    explicit AbstractCapoTransposeState(int capoFret)
        : m_capoFret(capoFret) {}

    virtual ~AbstractCapoTransposeState() = default;
    virtual std::shared_ptr<AbstractCapoTransposeState> transitionToPlaybackOnly() = 0;
    virtual std::shared_ptr<AbstractCapoTransposeState> transitionToStandardOnly() = 0;
    virtual std::shared_ptr<AbstractCapoTransposeState> transitionToTabOnly() = 0;
    virtual void setCapoFret(int fret)  = 0;

    void setTabPitchOffset(int v) { m_tabPitchOffset = v; }
    void setStandardPitchOffset(int v) { m_standardPitchOffset = v; }

    int tabPitchOffset() const { return m_tabPitchOffset; }
    int standardPitchOffset() const { return m_standardPitchOffset; }
    int capoFret() const { return m_capoFret; }

protected:
    int m_tabPitchOffset = 0;
    int m_standardPitchOffset = 0;
    int m_capoFret = 0;
};

class CapoTransposeStatePlaybackOnly : public AbstractCapoTransposeState
{
public:
    explicit CapoTransposeStatePlaybackOnly(int capoFret)
        : AbstractCapoTransposeState(capoFret) {}
    ~CapoTransposeStatePlaybackOnly() override = default;
    void setCapoFret(int fret) override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToPlaybackOnly() override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToStandardOnly() override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToTabOnly() override;
};

class CapoTransposeStateStandardOnly : public AbstractCapoTransposeState
{
public:
    explicit CapoTransposeStateStandardOnly(int capoFret)
        : AbstractCapoTransposeState(capoFret) {}
    ~CapoTransposeStateStandardOnly() override = default;
    void setCapoFret(int fret) override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToPlaybackOnly() override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToStandardOnly() override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToTabOnly() override;
};

class CapoTransposeStateTabOnly : public AbstractCapoTransposeState
{
public:
    explicit CapoTransposeStateTabOnly(int capoFret)
        : AbstractCapoTransposeState(capoFret) {}
    ~CapoTransposeStateTabOnly() override = default;
    void setCapoFret(int fret) override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToPlaybackOnly() override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToStandardOnly() override;
    std::shared_ptr<AbstractCapoTransposeState> transitionToTabOnly() override;
};
}

#endif // MU_ENGRAVING_CAPO_H
