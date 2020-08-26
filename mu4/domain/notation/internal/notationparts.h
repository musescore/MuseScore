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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_DOMAIN_NOTATION_NOTATIONPARTS_H
#define MU_DOMAIN_NOTATION_NOTATIONPARTS_H

#include "modularity/ioc.h"
#include "inotationparts.h"
#include "igetscore.h"
#include "scenes/instruments/iinstrumentsrepository.h"

namespace mu {
namespace domain {
namespace notation {
class NotationParts : public INotationParts
{
    INJECT(notation, scene::instruments::IInstrumentsRepository, instrumentsRepository)

public:
    NotationParts(IGetScore* getScore);

    PartList parts() const override;
    InstrumentList instrumentList(const QString& partId) const override;
    StaffList staffList(const QString& partId, const QString& instrumentId) const override;

    void setInstruments(const std::vector<QString>& instrumentTemplateIds) override;
    void setPartVisible(const QString& partId, bool visible) override;
    void setInstrumentVisible(const QString& partId, const QString& instrumentId, bool visible) override;

    void setStaffVisible(int staffIndex, bool visible) override;
    void setStaffType(int staffIndex, StaffType type) override;
    void setCutaway(int staffIndex, bool value) override;
    void setSmallStaff(int staffIndex, bool value) override;
    void setVoiceVisible(int staffIndex, int voiceIndex, bool value) override;

    void removeStaff(int staffIndex) override;
    void moveStaff(int fromIndex, int toIndex) override;

    Staff* appendStaff(const QString& partId, const QString& instrumentId) override;
    Staff* appendLinkedStaff(int staffIndex) override;

    async::Channel<Part*> partChanged() const override;
    async::Channel<Instrument*> instrumentChanged() const override;
    async::Channel<Staff*> staffChanged() const override;
    async::Notification partsChanged() const override;

private:
    Ms::Score* score() const;

    Part* part(const QString& id, Ms::Score* score = nullptr) const;
    Instrument* instrument(const QString& partId, const QString& instrumentId) const;
    Instrument* instrument(const Staff* staff) const;
    Staff* staff(int staffIndex) const;

    PartList scoreParts(Ms::Score* score) const;
    PartList excerptParts(Ms::Score* score) const;

    void appendPart(Part* part);
    void addStaves(Part* part, const scene::instruments::InstrumentTemplate& instrumentTemplate, int& globalStaffIndex);

    void insertInstrument(Part* part, Instrument* instrument, const StaffList& staves, const QString& beforeInstrumentId);

    void removeUnselectedInstruments(const scene::instruments::InstrumentTemplateList& instrumentTemplates);
    bool templatesContainsInstrument(const scene::instruments::InstrumentTemplateList& instrumentTemplates,
                                     const QString& instrumentId) const;
    std::vector<QString> missingInstrumentIds(const scene::instruments::InstrumentTemplateList& instrumentTemplates) const;

    void cleanEmptyExcerpts();

    Instrument instrumentFromTemplate(const scene::instruments::InstrumentTemplate& instrumentTemplate) const;
    void initStaff(Staff* staff, const scene::instruments::InstrumentTemplate& instrumentTemplate,const Ms::StaffType* staffType, int cidx);

    QList<Ms::NamedEventList> convertedMidiActions(const scene::instruments::MidiActionList& templateMidiActions) const;

    IGetScore* m_getScore = nullptr;

    async::Channel<Part*> m_partChanged;
    async::Channel<Instrument*> m_instrumentChanged;
    async::Channel<Staff*> m_staffChanged;
    async::Notification m_partsChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATION_NOTATIONPARTS_H
