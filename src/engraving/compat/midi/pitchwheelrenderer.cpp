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

    int32_t tick = functions.startTick;
    int prevPitchValue = _wheelSpec.mLimit;

    PitchWheelFunctions unProcessedFunctions = functions;

    for (; tick <= functions.endTick;) {
        int pitchValue = _wheelSpec.mLimit;

        PitchWheelFunctions processingFunctions;
        for (const auto& func : unProcessedFunctions.functions) {
            if (tick >= func.mStartTick) {
                processingFunctions.functions.push_back(func);
            }
        }

        if (processingFunctions.functions.empty()) {
            //find next tick
            tick = findNextStartTick(unProcessedFunctions.functions);
            continue;
        }

        auto iterBegin = unProcessedFunctions.functions.begin();
        auto iterEnd = unProcessedFunctions.functions.end();
        auto funcIter = iterBegin;
        while (funcIter != iterEnd) {
            if (tick > funcIter->mEndTick) {
                funcIter = unProcessedFunctions.functions.erase(funcIter);
            }

            pitchValue = calculatePitchBend(unProcessedFunctions.functions, tick);

            ++funcIter;
        }

        if (pitchValue != prevPitchValue) {
            NPlayEvent evb(ME_PITCHBEND, channel, pitchValue % 128, pitchValue / 128);
            evb.setEffect(effect);
            if (staffInfoValid) {
                evb.setOriginatingStaff(staffIdx);
            }

            pitchWheelEvents.emplace_hint(pitchWheelEvents.end(), std::make_pair(tick, evb));
        }

        prevPitchValue = pitchValue;
        tick += _wheelSpec.mStep;
    }

    //!@NOTE handling end of each function. endTick usually
    //! means end of note and starting of next note.
    //! But endTick can fall on the segment between two points.
    //! So next note can start with  pitch value of previous note.
    for (const auto& endFunc : functions.functions) {
        int pitchValue = _wheelSpec.mLimit;
        int32_t endFuncTick = endFunc.mEndTick;
        if (endFuncTick % _wheelSpec.mStep == 0) {
            //!already proccessed
            continue;
        }

        for (const auto& func : functions.functions) {
            if (endFuncTick < func.mStartTick || endFuncTick > func.mEndTick || &func == &endFunc) {
                continue;
            }

            pitchValue += func.func(endFuncTick);
        }

        if (endFuncTick == functions.endTick) {
            pitchValue = _wheelSpec.mLimit;
        }

        NPlayEvent evb(ME_PITCHBEND, channel, pitchValue % 128, pitchValue / 128);
        evb.setEffect(effect);
        if (staffInfoValid) {
            evb.setOriginatingStaff(staffIdx);
        }

        pitchWheelEvents.emplace_hint(pitchWheelEvents.end(), std::make_pair(endFuncTick, evb));
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
