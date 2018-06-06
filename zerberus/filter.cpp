//=============================================================================
//  Zerberus
//  Filters implementation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "filter.h"
#include "zerberus.h"
#include "zone.h"

#include <math.h>
#include <functional>

static constexpr int INTERP_MAX = 256;
static float interpCoeff[INTERP_MAX][4];

//---------------------------------------------------------
//   FilterBQ
//---------------------------------------------------------

ZFilter::ZFilter()
      {
      constexpr double ff = 1.0 / 32768.0;
      for (int i = 0; i < INTERP_MAX; i++) {
            double x = (double) i / (double) INTERP_MAX;
            interpCoeff[i][0] = (x * (-0.5 + x * (1 - 0.5 * x)))  * ff;
            interpCoeff[i][1] = (1.0 + x * x * (1.5 * x - 2.5))   * ff;
            interpCoeff[i][2] = (x * (0.5 + x * (2.0 - 1.5 * x))) * ff;
            interpCoeff[i][3] = (0.5 * x * x * (x - 1.0))         * ff;
            }
      }

//---------------------------------------------------------
//   initialize
//---------------------------------------------------------

void ZFilter::initialize(const Zerberus* zerberus, const Zone* z, int velocity)
      {
      this->zerberus = zerberus;
      this->sampleZone = z;

      resonanceF = zerberus->ct2hz(13500.0);
      if (z->isCutoffDefined) {
            //calculate current cutoff value
            float cutoffHz = z->cutoff;
            //Formula for converting the interval frequency ratio f2 / f1 to cents (c or ¢).
            //¢ or c = 1200 × log2 (f2 / f1)
            cutoffHz *= pow(2.0, velocity / 127.0f * z->fil_veltrack / 1200.0);
            resonanceF = cutoffHz;
            }

      last_resonanceF   = -1.0;
      float GEN_FILTERQ = 100.0;  // 0 - 960
      float q_db  = GEN_FILTERQ / 10.0f - 3.01f;
      q_lin       = pow(10.0f, q_db / 20.0f);
      gain = 1.0 / sqrt(q_lin);
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void ZFilter::update()
      {
      float adaptedFrequency = resonanceF;
      const float sr = zerberus->sampleRate();
      if (adaptedFrequency > 0.45f * sr)
            adaptedFrequency = 0.45f * sr;
      else if (adaptedFrequency < 5.f)
            adaptedFrequency = 5.f;

      const bool freqWasUpdated = fabs(adaptedFrequency - last_resonanceF) > 0.01f;
      if (!freqWasUpdated)
            return;

      last_resonanceF = adaptedFrequency;

      // The filter coefficients have to be recalculated (filter
      // parameters have changed). Recalculation for various reasons is
      // forced by setting last_fres to -1.  The flag filter_startup
      // indicates, that the DSP loop runs for the first time, in this
      // case, the filter is set directly, instead of smoothly fading
      // between old and new settings.
      //
      // Those equations from Robert Bristow-Johnson's `Cookbook
      // formulae for audio EQ biquad filter coefficients', obtained
      // from Harmony-central.com / Computer / Programming. They are
      // the result of the bilinear transform on an analogue filter
      // prototype. To quote, `BLT frequency warping has been taken
      // into account for both significant frequency relocation and for
      // bandwidth readjustment'.

      const float omega       = (float) 2.0f * M_PI * (adaptedFrequency / sr);
      const float sin_coeff   = sin(omega);
      const float cos_coeff   = cos(omega);
      const float alpha_coeff = sin_coeff / (2.0f * q_lin);
      const float a0_inv      = 1.0f / (1.0f + alpha_coeff);
      const float linCosCoeff = 2.f - cos_coeff;

      float a1_temp = 0.f;
      float a2_temp = 0.f;
      float b1_temp = 0.f;
      float b0_temp = 0.f;
      float b2_temp = 0.f;
      switch (sampleZone->fil_type) {
            case FilterType::lpf_2p: {
                  a1_temp = 2.0f * cos_coeff * a0_inv;
                  a2_temp = (alpha_coeff - 1.f) * a0_inv;
                  b1_temp = (1.0f - cos_coeff) * a0_inv * gain;
                  b0_temp = b2_temp = b1_temp * 0.5f;
                  break;
                  }
            case FilterType::hpf_2p: {
                  a1_temp = 2.0f * cos_coeff * a0_inv;
                  a2_temp = (alpha_coeff - 1.f) * a0_inv;
                  b1_temp = -(1.0f + cos_coeff) * a0_inv * gain;
                  b0_temp = b2_temp = -b1_temp * 0.5f;
                  break;
                  }
            case FilterType::bpf_2p: {
                  a1_temp = 2.0f * cos_coeff * a0_inv;
                  a2_temp = (alpha_coeff - 1.f) * a0_inv;
                  b0_temp = alpha_coeff * a0_inv;
                  b1_temp = 0.f;
                  b2_temp = -b0_temp;
                  break;
                  }
            case FilterType::brf_2p: {
                  a1_temp = 2.0f * cos_coeff * a0_inv;
                  a2_temp = (alpha_coeff - 1.f) * a0_inv;
                  b1_temp = a0_inv * (-2.f * cos_coeff);
                  b0_temp = b2_temp = a0_inv;
                  break;
                  }
            case FilterType::lpf_1p: {
                  a1_temp = -(linCosCoeff - sqrt(linCosCoeff * linCosCoeff - 1));
                  b0_temp = 1 + a1_temp;
                  break;
                  }
            case FilterType::hpf_1p: {
                  a1_temp = -(linCosCoeff - sqrt(linCosCoeff * linCosCoeff - 1));
                  b0_temp = -a1_temp;
                  b1_temp = a1_temp;
                  break;
                  }
            default:
                  qWarning() << "fil_type is not implemented: " << (int)sampleZone->fil_type;
            }

      if (firstRun) {
            /* The filter is calculated, because the voice was started up.
             * In this case set the filter coefficients without delay.
             */
            a1 = a1_temp;
            a2 = a2_temp;
            b0 = b0_temp;
            b2 = b2_temp;
            b1 = b1_temp;
            filter_coeff_incr_count = 0;
            firstRun = false;
            }
      else {
            /* The filter frequency is changed.  Calculate an increment
            * factor, so that the new setting is reached after some time.
            */

            static const int FILTER_TRANSITION_SAMPLES = 64;

            a1_incr = (a1_temp - a1) / FILTER_TRANSITION_SAMPLES;
            a2_incr = (a2_temp - a2) / FILTER_TRANSITION_SAMPLES;
            b0_incr = (b0_temp - b0) / FILTER_TRANSITION_SAMPLES;
            b1_incr = (b1_temp - b1) / FILTER_TRANSITION_SAMPLES;
            b2_incr = (b2_temp - b2) / FILTER_TRANSITION_SAMPLES;
            /* Have to add the increments filter_coeff_incr_count times. */
            filter_coeff_incr_count = FILTER_TRANSITION_SAMPLES;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

float ZFilter::apply(float inputValue, bool leftChannel)
      {
      float value = 0.f;
      const auto applyBqEquation = [this, &inputValue](float& histX1, float& histX2, float& histY1, float& histY2) {
            //apply filter
            /*
                  float y = d.b0 * x + d.b1 * d.x1 + d.b2 * d.x2 +
                            d.a1 * d.y1 + d.a2 * d.y2;
                  d.x2 = d.x1;
                  d.x1 = x;
                  d.y2 = d.y1;
                  d.y1 = y;
                  return y;
             */
            float value = b0 * inputValue + b1 * histX1 + b2 * histX2 + a1 * histY1 + a2 * histY2;
            histX2 = histX1;
            histX1 = inputValue;
            histY2 = histY1;
            histY1 = value;
            return value;
            };
      const auto applyHPF1PEquation = [this, &inputValue](float& histX1, float& histY1) {
            float value = b0 * inputValue + b1 * histX1 - a1 * histY1;
            histX1 = inputValue;
            histY1 = value;
            return value;
            };
      const auto applyLPF1PEquation = [this, &inputValue](float& histY1) {
            float value = b0 * inputValue - a1 * histY1;
            histY1 = value;
            return value;
            };
      if (leftChannel) {
            switch (sampleZone->fil_type) {
                  case FilterType::hpf_2p:
                  case FilterType::lpf_2p:
                  case FilterType::bpf_2p:
                  case FilterType::brf_2p: {
                        value = applyBqEquation(monoL.histX1, monoL.histX2, monoL.histY1, monoL.histY2);
                        break;
                        }
                  case FilterType::hpf_1p: {
                        value = applyHPF1PEquation(monoL.histX1, monoL.histY1);
                        break;
                        }
                  case FilterType::lpf_1p: {
                        value = applyLPF1PEquation(monoL.histY1);
                        break;
                        }
                  default:
                        qWarning() << "this equation is not implemented" << (int)sampleZone->fil_type;
                  }
            }
      else { //right channel in stereo samples
            switch (sampleZone->fil_type) {
                  case FilterType::hpf_2p:
                  case FilterType::lpf_2p:
                  case FilterType::bpf_2p:
                  case FilterType::brf_2p: {
                        value = applyBqEquation(monoR.histX1, monoR.histX2, monoR.histY1, monoR.histY2);
                        break;
                        }
                  case FilterType::hpf_1p: {
                        value = applyHPF1PEquation(monoR.histX1, monoR.histY1);
                        break;
                        }
                  case FilterType::lpf_1p: {
                        value = applyLPF1PEquation(monoR.histY1);
                        break;
                        }
                  default:
                        qWarning() << "this equation is not implemented" << (int)sampleZone->fil_type;
                  }
            }

      if (filter_coeff_incr_count) {
            --filter_coeff_incr_count;
            a1 += a1_incr;
            a2 += a2_incr;
            b0 += b0_incr;
            b1 += b1_incr;
            b2 += b2_incr;
            }

      return value;
      }

//---------------------------------------------------------
//   interpolate
//---------------------------------------------------------

float ZFilter::interpolate(unsigned phase, short prevVal, short currVal, short nextVal, short nextNextVal) const
      {
      const auto& interpValTable = interpCoeff[phase];
      return (interpValTable[0] * prevVal
          + interpValTable[1] * currVal
          + interpValTable[2] * nextVal
          + interpValTable[3] * nextNextVal);
      }
