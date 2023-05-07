#include "pitchwheelrenderer.h"

#include "log.h"

using namespace mu::engraving;

PitchWheelRenderer::PitchWheelRenderer(PitchWheelSpecs wheelSpec)
    : _wheelSpec(wheelSpec)
{}

void PitchWheelRenderer::addPitchWheelFunction(const PitchWheelFunction& function, uint32_t channel, MidiInstrumentEffect effect)
{
    PitchWheelFunctions& functions =  _functions[channel];
    _effectByChannel[channel] = effect;

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

            ++funcIter;
        }

        PitchWheelFunctions pitchBendCalculationFunctions;
        for (const auto& func : unProcessedFunctions.functions) {
            if (tick >= func.mStartTick && tick < func.mEndTick) {
                pitchBendCalculationFunctions.functions.push_back(func);
            }
        }

        pitchValue = calculatePitchBend(pitchBendCalculationFunctions.functions, tick);

        if (pitchValue != prevPitchValue) {
            if (!pitchBendCalculationFunctions.functions.empty()) {
                /// handling pitchbend from previous ticks
                auto lastPitchInfoIt = m_lastPitchInfoByChan.find(channel);
                if (lastPitchInfoIt != m_lastPitchInfoByChan.end()) {
                    int oldTick = m_lastPitchInfoByChan.at(channel).tick;
                    int ticksDist = tick - oldTick;
                    int oldPitch = m_lastPitchInfoByChan.at(channel).pitch;
                    NPlayEvent evb(ME_PITCHBEND, channel, oldPitch % 128, oldPitch / 128);
                    evb.setEffect(effect);
                    pitchWheelEvents.emplace_hint(pitchWheelEvents.end(), std::make_pair(oldTick + ticksDist / 2, evb));

                    m_lastPitchInfoByChan.erase(lastPitchInfoIt);
                }

                NPlayEvent evb(ME_PITCHBEND, channel, pitchValue % 128, pitchValue / 128);
                evb.setEffect(effect);
                pitchWheelEvents.emplace_hint(pitchWheelEvents.end(), std::make_pair(tick, evb));
            } else {
                /// save the value for next
                PitchBendInfo pbInfo;
                pbInfo.tick = tick;
                pbInfo.pitch = pitchValue;
                m_lastPitchInfoByChan[channel] = pbInfo;
            }
        }

        prevPitchValue = pitchValue;
        tick += _wheelSpec.mStep;
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
        pitchValue += func.func(tick);
    }

    return pitchValue;
}
