/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "fluid.h"
#include "sfont.h"
#include "gen.h"

namespace FluidS {

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

Channel::Channel(Fluid* s, int num)
      {
      synth   = s;
      channum = num;
      _preset = 0;
      banknum = 0;
      prognum = 0;
      reset();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Channel::reset()
      {
      init();
      initCtrl();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Channel::init()
      {
      sfontnum      = 0;
      setPreset(synth->find_preset(banknum, prognum));
      interp_method = FLUID_INTERP_DEFAULT;
      nrpn_select   = 0;
      }

//---------------------------------------------------------
//   initCtrl
//---------------------------------------------------------

void Channel::initCtrl()
      {
      channel_pressure = 0;
      pitch_bend       = 0x2000; // Range is 0x4000, pitch bend wheel starts in centered position
      pitch_wheel_sensitivity = 12; /* twelve semi-tones */
      bank_msb         = 0;

      for (int i = 0; i < GEN_LAST; i++) {
            gen[i]     = 0.0f;
            gen_abs[i] = 0;
            }

      /* Reset key pressure and CCs for all possible values */
      for (int i = 0; i < 128; i++) {
            // For MuseScore purposes, default to 80 for poly aftertouch
            setKeyPressure(i, 80);
            setCC(i, 0);
            }

      /* Volume / initial attenuation (MSB & LSB) */
      setCC(VOLUME_MSB, 127);
      setCC(VOLUME_LSB, 0);

      /* Pan (MSB & LSB) */
      setCC(PAN_MSB, 64);
      setCC(PAN_LSB, 64);

      /* Expression (MSB & LSB) */
      setCC(EXPRESSION_MSB, 127);
      setCC(EXPRESSION_LSB, 127);
      }

//---------------------------------------------------------
//   setcc
//---------------------------------------------------------

void Channel::setcc(int num, int value)
      {
      cc[num] = value;

//printf("setcc %d %d\n", num, value);

      switch (num) {
            case SUSTAIN_SWITCH:
                  if (value < 64)
                        synth->damp_voices(channum);
                  break;

            case BANK_SELECT_MSB:
                  {
                  bank_msb = value & 0x7f;
                  setBanknum(bank_msb << 7);
                  }
                  break;

            case BANK_SELECT_LSB:
                  /* FIXME: according to the Downloadable Sounds II specification,
                     bit 31 should be set when we receive the message on channel
                     10 (drum channel) */
                  setBanknum((value & 0x7f) + (bank_msb << 7));
                  break;

            case ALL_NOTES_OFF:
                  synth->allNotesOff(channum);
                  break;

            case ALL_SOUND_OFF:
                  synth->allSoundsOff(channum);
                  break;

            case ALL_CTRL_OFF:
                  initCtrl();
                  synth->modulate_voices_all(channum);
                  break;

            case DATA_ENTRY_MSB:
                  {
                  int data = (value << 7) + cc[DATA_ENTRY_LSB];

                  /* SontFont 2.01 NRPN Message (Sect. 9.6, p. 74)  */
                  if ((cc[NRPN_MSB] == 120) && (cc[NRPN_LSB] < 100)) {
                        float val = fluid_gen_scale_nrpn(nrpn_select, data);
                        qDebug("%s: %d: Data = %d, value = %f", __FILE__, __LINE__, data, val);
                        synth->set_gen(channum, nrpn_select, val);
                        }
                  break;
                  }

            case NRPN_MSB:
                  cc[NRPN_LSB] = 0;
                  nrpn_select = 0;
                  break;

            case NRPN_LSB:
                  /* SontFont 2.01 NRPN Message (Sect. 9.6, p. 74)  */
                  if (cc[NRPN_MSB] == 120) {
                        if (value == 100)
                              nrpn_select += 100;
                        else if (value == 101)
                              nrpn_select += 1000;
                        else if (value == 102)
                              nrpn_select += 10000;
                        else if (value < 100) {
                              nrpn_select += value;
                              qDebug("%s: %d: NRPN Select = %d", __FILE__, __LINE__, nrpn_select);
                              }
                        }
                  break;

            case RPN_MSB:
                  break;

            case RPN_LSB:
                  /* erase any previously received NRPN message  */
                  cc[NRPN_MSB] = 0;
                  cc[NRPN_LSB] = 0;
                  nrpn_select  = 0;
                  break;

            default:
                  synth->modulate_voices(channum, true, num);
            }
      }

//---------------------------------------------------------
//   getCC
//---------------------------------------------------------

int Channel::getCC(int num)
      {
      return ((num >= 0) && (num < 128))? cc[num] : 0;
      }

int Channel::getFromKeyPortamento() {
      if (synth)
            return synth->getFromKeyPortamento();
      return -1;
      }

//---------------------------------------------------------
//   pitchBend
//---------------------------------------------------------

void Channel::pitchBend(int val)
      {
      pitch_bend = val;
      synth->modulate_voices(channum, false, FLUID_MOD_PITCHWHEEL);
      }

//---------------------------------------------------------
//   setChannelPressure
//---------------------------------------------------------

void Channel::setChannelPressure(int val)
      {
      channel_pressure = val;
      synth->modulate_voices(channum, false, FLUID_MOD_CHANNELPRESSURE);
      }

//---------------------------------------------------------
//   setKeyPressure
//---------------------------------------------------------

void Channel::setKeyPressure(int key, int val)
      {
      key_pressure[key] = val;
      synth->modulate_voices(channum, false, FLUID_MOD_KEYPRESSURE);
      }

//---------------------------------------------------------
//   pitchWheelSens
//---------------------------------------------------------

void Channel::pitchWheelSens(int val)
      {
      pitch_wheel_sensitivity = val;
      synth->modulate_voices(channum, false, FLUID_MOD_PITCHWHEELSENS);
      }

//---------------------------------------------------------
//   setPreset
//---------------------------------------------------------

void Channel::setPreset(Preset* p)
      {
      if (_preset != p) {
            if (p)
                  p->loadSamples();
            _preset = p;
            }
      }

}

