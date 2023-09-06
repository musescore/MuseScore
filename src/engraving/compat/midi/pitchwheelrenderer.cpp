#include "pitchwheelrenderer.h"

#include "log.h"

using namespace mu::engraving;

PitchWheelRenderer::PitchWheelRenderer(PitchWheelSpecs wheelSpec)
    : _wheelSpec(wheelSpec)
{}

void PitchWheelRenderer::addPitchWheelFunction(const PitchWheelFunction& function, uint32_t channel, staff_idx_t staffIdx,
                                               MidiInstrumentEffect effect)
{
    PitchWheelFunctions& functions =  _functions[channel];
    _effectByChannel[channel] = effect;
    _staffIdxByChannel[channel] = staffIdx;

    if (function.mStartTick < functions.startTick) {
        functions.startTick = function.mStartTick;
    }
    if (function.mEndTick > functions.endTick) {
        functions.endTick = function.mEndTick;
    }

    functions.functions.push_back(function);
}

EventMap PitchWheelRenderer::renderPitchWheel() const noexcept
{
    EventMap pitchWheelEvents;

    for (const auto& function : _functions) {
        renderChannelPitchWheel(pitchWheelEvents, function.second, function.first);
    }

    return pitchWheelEvents;
}

//! MARK: PRIVATE METHODS

void PitchWheelRenderer::renderChannelPitchWheel(EventMap& pitchWheelEvents,
                                                 const PitchWheelFunctions& functions,
                                                 uint32_t channel) const noexcept
{
    if (functions.endTick < functions.startTick) {
        return;
    }

    MidiInstrumentEffect effect = MidiInstrumentEffect::NONE;
    if (_effectByChannel.find(channel) != _effectByChannel.end()) {
        effect = _effectByChannel.at(channel);
    }

    bool staffInfoValid = false;
    staff_idx_t staffIdx = 0;

    if (_staffIdxByChannel.find(channel) != _staffIdxByChannel.end()) {
        staffInfoValid = true;
        staffIdx = _staffIdxByChannel.at(channel);
    }

    std::map<int, int, std::greater<> > ranges;
    generateRanges(functions.functions, ranges);

    for (auto rit = ranges.crbegin(); rit != ranges.crend(); ++rit) {
        int32_t start = rit->first;
        int32_t end = rit->second;
        int32_t tick = start;

        std::list<PitchWheelFunction> functionsToProcess;
        for (const auto& func : functions.functions) {
            if (func.mEndTick <= end) {
                functionsToProcess.insert(functionsToProcess.end(), func);
            }
        }
        std::vector<int> pitches;
        for (size_t i = 0; i < functionsToProcess.size(); ++i) {
            pitches.push_back(0);
        }
        int prevPitch = _wheelSpec.mLimit;
        bool forceUpdate = false;
        while (tick < end) {
            auto funcIt = functionsToProcess.begin();
            for (size_t i = 0; i < functionsToProcess.size(); ++i) {
                if (tick >= funcIt->mEndTick) {
                    // Function exceeds its max range
                    // don't need its value anymore
                    pitches.at(i) = 0;
                    ++funcIt;
                    forceUpdate = true;
                    continue;
                }
                if (tick < funcIt->mStartTick) {
                    ++funcIt;
                    continue;
                }
                int pitch = funcIt->func(tick);
                pitches.at(i) = pitch;
                ++funcIt;
            }
            int finalPitch = _wheelSpec.mLimit;
            for (const auto& pitch : pitches) {
                finalPitch += pitch;
            }
            if (forceUpdate || finalPitch != prevPitch || tick == start) {
                NPlayEvent evb(ME_PITCHBEND, channel, finalPitch % 128, finalPitch / 128);
                evb.setEffect(effect);
                if (staffInfoValid) {
                    evb.setOriginatingStaff(staffIdx);
                }
                pitchWheelEvents[channel].emplace_hint(pitchWheelEvents[channel].end(), std::make_pair(tick, evb));
                forceUpdate = false;
            }

            prevPitch = finalPitch;
            tick += _wheelSpec.mStep;
        }
    }
}

int32_t PitchWheelRenderer::findNextStartTick(const std::list<PitchWheelFunction>& functions) const noexcept
{
    int32_t tick = std::numeric_limits<int32_t>::max();
    for (const auto& func : functions) {
        tick = std::min(tick, func.mStartTick);
    }

    return tick;
}

int32_t PitchWheelRenderer::calculatePitchBend(const std::list<PitchWheelFunction>& functions, int32_t tick) const noexcept
{
    int pitchValue = _wheelSpec.mLimit;

    for (const auto& func : functions) {
        if (tick < func.mStartTick || tick >= func.mEndTick) {
            continue;
        }

        pitchValue += func.func(tick);
    }

    return pitchValue;
}

// 1                |------|
// 2                   |-----------| handleStartTick();
// result           |--------------|
// 3           |--------|            handleEndTick();
// result      |-------------------|
void PitchWheelRenderer::generateRanges(const std::list<PitchWheelFunction>& functions, std::map<int, int, std::greater<> >& ranges)
{
    // !NOTE ranges map is reversed. Use reverse iterators
    auto handleEndTick = [&](const PitchWheelFunction& func) {
        auto lowerRange = ranges.upper_bound(func.mEndTick);
        if (lowerRange == ranges.end()) {
            return false;
        }
        ranges.insert({ func.mStartTick, lowerRange->second });
        ranges.erase(lowerRange);
        return true;
    };

    auto handleStartTick = [&](const PitchWheelFunction& func) {
        auto lowerRange = ranges.upper_bound(func.mStartTick);
        if (lowerRange == ranges.end()) {
            // We are getting PW events sorted by startTick
            // So in ideal world handleEndTick always return false
            return handleEndTick(func);
        }
        if (lowerRange->second >= func.mStartTick) {
            lowerRange->second = std::max(lowerRange->second, func.mEndTick);
            return true;
        }
        return false;
    };

    for (const auto& func : functions) {
        if (auto key = ranges.find(func.mStartTick); key != ranges.end()) {
            key->second = std::max(key->second, func.mEndTick);
            continue;
        }
        if (!handleStartTick(func)) {
            ranges.insert({ func.mStartTick, func.mEndTick });
        }
    }
}
