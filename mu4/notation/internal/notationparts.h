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
#ifndef MU_NOTATION_NOTATIONPARTS_H
#define MU_NOTATION_NOTATIONPARTS_H

#include "inotationparts.h"
#include "igetscore.h"

namespace mu {
namespace notation {
class NotationParts : public INotationParts
{
public:
    NotationParts(IGetScore* getScore);

    PartList partList() const override;
    instruments::InstrumentList instrumentList(const QString& partId) const override;
    StaffList staffList(const QString& partId, const QString& instrumentId) const override;

    void setInstruments(const instruments::InstrumentList& instruments) override;
    void setPartVisible(const QString& partId, bool visible) override;
    void setPartName(const QString& partId, const QString& name) override;
    void setInstrumentVisible(const QString& partId, const QString& instrumentId, bool visible) override;
    void setInstrumentName(const QString& partId, const QString& instrumentId, const QString& name) override;
    void setInstrumentAbbreviature(const QString& partId, const QString& instrumentId, const QString& abbreviature) override;
    void setStaffVisible(int staffIndex, bool visible) override;
    void setVoiceVisible(int staffIndex, int voiceIndex, bool visible) override;
    void setStaffType(int staffIndex, StaffType type) override;
    void setCutaway(int staffIndex, bool value) override;
    void setSmallStaff(int staffIndex, bool value) override;

    void removeParts(const std::vector<QString>& partsIds) override;
    void removeInstruments(const QString& partId, const std::vector<QString>& instrumentIds) override;
    void removeStaves(const std::vector<int>& stavesIndexes) override;

    void movePart(const QString& partId, const QString& toPartId, InsertMode mode = Before) override;
    void moveInstrument(const QString& instrumentId, const QString& fromPartId, const QString& toPartId,const QString& toInstrumentId,
                        InsertMode mode = Before) override;
    void moveStaff(int staffIndex, int toStaffIndex, InsertMode mode = Before) override;

    const Staff* appendStaff(const QString& partId, const QString& instrumentId) override;
    const Staff* appendLinkedStaff(int staffIndex) override;

    void replaceInstrument(const QString& partId, const QString& instrumentId, const instruments::Instrument& newInstrument) override;

    async::Channel<const Part*> partChanged() const override;
    async::Channel<const instruments::Instrument> instrumentChanged() const override;
    async::Channel<const Staff*> staffChanged() const override;
    async::Notification partsChanged() const override;

private:
    struct InstrumentInfo
    {
        int tick = 0;
        Ms::Instrument* instrument = nullptr;

        InstrumentInfo() = default;

        InstrumentInfo(int tick, Ms::Instrument* instrument)
            : tick(tick), instrument(instrument) {}

        bool isValid() const { return instrument != nullptr; }
    };

    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;

    void startEdit();
    void apply();

    void doSetStaffVisible(Staff* staff, bool visible);
    void doRemoveParts(const std::vector<QString>& partsIds);
    void doRemoveInstruments(Part* part, const std::vector<QString>& instrumentIds);
    void doRemoveStaves(const std::vector<int>& stavesIndexes);

    Part* part(const QString& partId, const Ms::Score* score = nullptr) const;
    InstrumentInfo instrumentInfo(const Part* part, const QString& instrumentId) const;
    InstrumentInfo instrumentInfo(const Staff* staff) const;
    Staff* staff(int staffIndex) const;

    StaffList staves(const Part* part, const QString& instrumentId) const;

    QList<Part*> scoreParts(const Ms::Score* score) const;
    QList<Part*> excerptParts(const Ms::Score* score) const;

    void appendPart(Part* part);
    void addStaves(Part* part, const instruments::Instrument& instrument, int& globalStaffIndex);

    void insertInstrument(Part* part, Ms::Instrument* instrumentInfo, const StaffList& staves, const QString& toInstrumentId,
                          InsertMode mode);

    void removeUnselectedInstruments(const std::vector<QString>& selectedInstrumentIds);
    std::vector<QString> missingInstrumentIds(const std::vector<QString>& selectedInstrumentIds) const;

    void cleanEmptyExcerpts();

    Ms::Instrument museScoreInstrument(const instruments::Instrument& instrument) const;
    instruments::Instrument instrument(Ms::Instrument* museScoreInstrument) const;

    void initStaff(Staff* staff, const instruments::Instrument& instrument, const Ms::StaffType* staffType, int cidx);

    QList<Ms::NamedEventList> convertedMidiActions(const instruments::MidiActionList& midiActions) const;
    instruments::MidiActionList convertedMidiActions(const QList<Ms::NamedEventList>& midiActions) const;

    IGetScore* m_getScore = nullptr;

    async::Channel<const Part*> m_partChanged;
    async::Channel<const instruments::Instrument> m_instrumentChanged;
    async::Channel<const Staff*> m_staffChanged;
    async::Notification m_partsChanged;
};
}
}

#endif // MU_NOTATION_NOTATIONPARTS_H
