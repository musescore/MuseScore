/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "messages.h"
#include "aeolus.h"

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void Aeolus::audio_start ()
      {
      _audio->_nasect = _nasect;
      _audio->_fsamp  = _fsamp;
      _audio->_fsize  = 0;
      _audio->_instrpar = _audiopar;
      for (int i = 0; i < _nasect; i++)
            _audio->_asectpar [i] = _asectp [i]->get_apar ();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Aeolus::audio_init(int sampleRate)
      {
	_nplay   = 2;
      _fsamp   = sampleRate;
      _audiopar[VOLUME].set("volume", 0.32f, 0.00f, 1.00f);
      _audiopar[REVSIZE].set("revsize", 0.075f, 0.025f, 0.150f);
      _revtime = 4.0f;
      _audiopar[REVTIME].set("revtime", _revtime, 2.0f, 7.0f);
      _audiopar[STPOSIT].set("stposit", 0.5f, -1.0f, 1.0f);

      _reverb.init (_fsamp);
      _reverb.set_t60mf (_revtime);
      _reverb.set_t60lo (_revtime * 1.50f, 250.0f);
      _reverb.set_t60hi (_revtime * 0.50f, 3e3f);

      _nasect = NASECT;
      for (int i = 0; i < NASECT; i++) {
            _asectp [i] = new Asection ((float) _fsamp);
            _asectp [i]->set_size (_revsize);
            }
      _hold = KEYS_MASK;
      }

//---------------------------------------------------------
//   proc_queue
//    Execute command from the model
//---------------------------------------------------------

void Aeolus::proc_queue (uint32_t k)
      {
      int c = k >> 24;
      int j = (k >> 16) & 255;
      int i = (k >>  8) & 255;
      int b = k & 255;

      switch (c) {
            case 0:     // Single key off.
                  key_off (i, b);
                  break;

            case 1:     // Single key on.
                  key_on (i, b);
	            break;

            case 2:     // Conditional key off.
                  cond_key_off (j, b);
                  break;

            case 3:     // Conditional key on.
                  cond_key_on (j, b);
                  break;

            case 4:     // Clear bits in division mask.
                  _divisp [j]->clr_div_mask (b);
                  break;

            case 5:     // Set bits in division mask.
                  _divisp [j]->set_div_mask (b);
                  break;

            case 6:     // Clear bits in rank mask.
                  _divisp [j]->clr_rank_mask (i, b);
                  break;

            case 7:     // Set bits in rank mask.
                  _divisp [j]->set_rank_mask (i, b);
                  break;

            case 8:     // Hold off.
                  _hold = KEYS_MASK;
                  cond_key_off (HOLD_MASK, HOLD_MASK);
                  break;

            case 9:     // Hold on.
                  _hold = KEYS_MASK | HOLD_MASK;
                  cond_key_on (j, HOLD_MASK);
                  break;

            case 16:    // Tremulant on/off.
                  if (b)
                        _divisp [j]->trem_on ();
                  else
                        _divisp [j]->trem_off ();
                  break;

            case 17:    // Per-division performance controllers.
#if 0
                  if (n < 2)
                        return;
//TODO                  u.i = Q->read (1);
//TODO                  Q->read_commit (2);
                  switch (i) {
                        case 0: _divisp [j]->set_swell (u.f); break;
                        case 1: _divisp [j]->set_tfreq (u.f); break;
                        case 2: _divisp [j]->set_tmodd (u.f); break;
                        break;
                        }
#endif
                  break;
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Aeolus::process(unsigned nframes, float* out, float gain)
      {
      for (int n = 0; n < NNOTES; n++) {
            int m = _keymap[n];
            if (m & 128) {
                  m &= 127;
                  _keymap [n] = m;
                  for (int d = 0; d < _ndivis; d++)
                        _divisp[d]->update (n, m);
	            }
            }
      for (int d = 0; d < _ndivis; d++)
            _divisp[d]->update(_keymap);

      if (fabsf(_revsize - _audiopar [REVSIZE].fval()) > 0.001f) {
            _revsize = _audiopar[REVSIZE].fval();
            _reverb.set_delay(_revsize);
            for (int j = 0; j < _nasect; j++)
                  _asectp[j]->set_size(_revsize);
            }
      if (fabsf(_revtime - _audiopar [REVTIME].fval()) > 0.1f) {
            _revtime = _audiopar [REVTIME].fval();
            _reverb.set_t60mf(_revtime);
            _reverb.set_t60lo(_revtime * 1.50f, 250.0f);
            _reverb.set_t60hi(_revtime * 0.50f, 3e3f);
            }

      int k = nout;
      while (nframes > 0) {
            if (nout == 0) {
                  float W [PERIOD];
                  float X [PERIOD];
                  float Y [PERIOD];
                  float R [PERIOD];
                  memset(W, 0, PERIOD * sizeof (float));
                  memset(X, 0, PERIOD * sizeof (float));
                  memset(Y, 0, PERIOD * sizeof (float));
                  memset(R, 0, PERIOD * sizeof (float));

                  for (int j = 0; j < _ndivis; j++)
                        _divisp[j]->process();
                  for (int j = 0; j < _nasect; j++)
                        _asectp[j]->process(gain, W, X, Y, R);

                  _reverb.process(PERIOD, gain, R, W, X, Y);

                  float stposit = _audiopar[STPOSIT].fval();
                  for (int j = 0; j < PERIOD; j++) {
                        loutb[j] = W[j] + stposit * X[j] + Y[j];
                        routb[j] = W[j] + stposit * X[j] - Y[j];
                        }
                  nout = PERIOD;
                  k += PERIOD;
                  }
            *out++ += gain * loutb[PERIOD - nout];
            *out++ += gain * routb[PERIOD - nout];
            --nout;
            --nframes;
            }
      }

//---------------------------------------------------------
//   newDivis
//---------------------------------------------------------

void Aeolus::newDivis(M_new_divis* X)
      {
      Division     *D = new Division (_asectp [X->_asect], (float) _fsamp);
      D->set_div_mask (X->_dmask);
      D->set_swell (X->_swell);
      D->set_tfreq (X->_tfreq);
      D->set_tmodd (X->_tmodd);
      _divisp [_ndivis] = D;
      _ndivis++;
      }

void Aeolus::cond_key_off (int m, int b)
      {
      unsigned char* p = _keymap;

      for (int i = 0; i < NNOTES; i++, p++) {
            if (*p & m) {
                  *p &= ~b;
                  *p |= 128;
                  }
            }
      }

void Aeolus::cond_key_on (int m, int b)
      {
      unsigned char* p = _keymap;

      for (int i = 0; i < NNOTES; i++, p++) {
            if (*p & m)
                  *p |= b | 128;
            }
      }

void Aeolus::key_off (int n, int b)
      {
      _keymap[n] &= ~b;
      _keymap[n] |= 128;
      }

void Aeolus::key_on (int n, int b)
      {
      _keymap[n] |= b | 128;
      }


