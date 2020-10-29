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
#include "async/asyncable.h"

namespace mu {
namespace notation {
class IGetScore;
class NotationParts : public INotationParts, public async::Asyncable
{
public:
    NotationParts(IGetScore* getScore, mu::async::Notification selectionChangedNotification);
    ~NotationParts() override;

    async::NotifyList<const Part*> partList() const override;
    async::NotifyList<instruments::Instrument> instrumentList(const ID& partId) const override;
    async::NotifyList<const Staff*> staffList(const ID& partId, const ID& instrumentId) const override;

    ValCh<bool> canChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const override;

    void setInstruments(const instruments::InstrumentList& instruments) override;
    void setPartVisible(const ID& partId, bool visible) override;
    void setInstrumentVisible(const ID& instrumentId, const ID& fromPartId, bool visible) override;
    void setStaffVisible(const ID& staffId, bool visible) override;
    void setVoiceVisible(const ID& staffId, int voiceIndex, bool visible) override;
    void setPartName(const ID& partId, const QString& name) override;
    void setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name) override;
    void setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature) override;
    void setStaffType(const ID& staffId, StaffType type) override;
    void setCutawayEnabled(const ID& staffId, bool enabled) override;
    void setSmallStaff(const ID& staffId, bool smallStaff) override;

    void removeParts(const IDList& partsIds) override;
    void removeInstruments(const IDList& instrumentsIds, const ID& fromPartId) override;
    void removeStaves(const IDList& stavesIds) override;

    void moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode = Before) override;
    void moveInstruments(const IDList& sourceInstrumentIds, const ID& sourcePartId, const ID& destinationPartId,
                         const ID& destinationInstrumentId, InsertMode mode = Before) override;
    void moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode = Before) override;

    void appendDoublingInstrument(const instruments::Instrument& instrument, const ID& destinationPartId) override;
    void appendStaff(const ID& destinationPartId) override;
    void appendLinkedStaff(const ID& originStaffId) override;

    void replaceInstrument(const ID& instrumentId, const ID& fromPartId, const instruments::Instrument& newInstrument) override;

    async::Notification partsChanged() const override;

private:
    struct InstrumentInfo
    {
        Ms::Fraction fraction = { -1, -1 };
        Ms::Instrument* instrument = nullptr;

        InstrumentInfo() = default;

        InstrumentInfo(const Ms::Fraction& fraction, Ms::Instrument* instrument)
            : fraction(fraction), instrument(instrument) {}

        bool isValid() const { return instrument != nullptr; }
    };

    struct InstrumentKey
    {
        ID partId;
        ID instrumentId;

        bool operator==(const InstrumentKey& key) const
        {
            return partId == key.partId && instrumentId == key.instrumentId;
        }

        friend uint qHash(const InstrumentKey& key)
        {
            return qHash(QString(key.partId + key.instrumentId));
        }
    };

    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;

    void startEdit();
    void apply();

    Ms::ChordRest* selectedChord() const;
    void updateCanChangeInstrumentsVisibility();
    bool resolveCanChangeInstrumentVisibility(const ID& instrumentId, const ID& fromPartId) const;
    bool needAssignInstrumentToChord(const ID& instrumentId, const ID& fromPartId) const;
    void assignIstrumentToSelectedChord(Ms::Instrument* instrument);

    void doMovePart(const ID& sourcePartId, const ID& destinationPartId, InsertMode mode = Before);
    void doSetStaffVisible(Staff* staff, bool visible);
    void doRemoveParts(const IDList& partsIds);
    void doRemoveInstruments(const IDList& instrumentsIds, Part* fromPart);
    void doSetPartName(Part* part, const QString& name);

    Part* part(const ID& partId, const Ms::Score* score = nullptr) const;
    InstrumentInfo instrumentInfo(const ID& instrumentId, const Part* fromPart) const;
    InstrumentInfo instrumentInfo(const Staff* staff) const;

    Staff* staff(const ID& staffId) const;
    std::vector<const Staff*> staves(const Part* part, const ID& instrumentId) const;
    std::vector<Staff*> staves(const IDList& stavesIds) const;

    std::vector<Part*> availableParts() const;
    std::vector<Part*> scoreParts(const Ms::Score* score) const;
    std::vector<Part*> excerptParts(const Ms::Score* score) const;

    void appendPart(Part* part);
    void appendStaves(Part* part, const instruments::Instrument& instrument);

    void removeMissingInstruments(const IDList& selectedInstrumentIds);
    IDList allInstrumentsIds() const;
    int lastStaffIndex() const;

    void removeEmptyExcerpts();

    Ms::Instrument convertedInstrument(const instruments::Instrument& instrument) const;
    instruments::Instrument convertedInstrument(const Ms::Instrument* museScoreInstrument, const Part* part) const;

    bool isInstrumentVisible(const ID& instrumentId, const Part* fromPart) const;

    void initStaff(Staff* staff, const instruments::Instrument& instrument, const Ms::StaffType* staffType, int cleffIndex);

    QList<Ms::NamedEventList> convertedMidiActions(const instruments::MidiActionList& midiActions) const;
    instruments::MidiActionList convertedMidiActions(const QList<Ms::NamedEventList>& midiActions) const;

    void sortParts(const IDList& instrumentIds);

    void notifyAboutStaffChanged(const ID& staffId) const;
    void notifyAboutInstrumentsChanged(const ID& partId) const;

    async::ChangedNotifier<instruments::Instrument>* partNotifier(const ID& partId) const;
    async::ChangedNotifier<const Staff*>* instrumentNotifier(const ID& instrumentId, const ID& fromPartId) const;

    QString formatPartName(const Part* part) const;
    QMap<Ms::Fraction, Ms::InstrumentChange*> instrumentChangeElements(const QString& partId) const;
    Ms::ChordRest* chordRest(const Ms::Fraction& fraction, const Part* fromPart) const;

    QMap<Ms::Fraction, Ms::Instrument*> instruments(const Part* fromPart, const IDList& filterInstrumentsIds = IDList()) const;
    void doInsertInstruments(const QMap<Ms::Fraction, Ms::Instrument*>& instruments, const ID& destinationPartId,
                             const ID& destinationInstrumentId, InsertMode mode = Before);

    IGetScore* m_getScore = nullptr;
    async::Notification m_partsChanged;

    mutable async::ChangedNotifier<const Part*>* m_partsNotifier = nullptr;
    mutable std::map<ID, async::ChangedNotifier<instruments::Instrument>*> m_partsNotifiersMap;
    mutable QHash<InstrumentKey, async::ChangedNotifier<const Staff*>*> m_instrumentsNotifiersHash;
    mutable QHash<InstrumentKey, ValCh<bool> > m_canChangeInstrumentsVisibilityHash;
};
}
}

namespace Ms {
inline uint qHash(const Ms::Fraction& fraction)
{
    return qHash(QString::number(fraction.ticks()));
}
}

#endif // MU_NOTATION_NOTATIONPARTS_H
