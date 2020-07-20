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


/* Purpose:
 * Low-level voice processing:
 *
 * - interpolates (obtains values between the samples of the original waveform data)
 * - filters (applies a lowpass filter with variable cutoff frequency and quality factor)
 * - mixes the processed sample to left and right output using the pan setting
 * - sends the processed sample to chorus and reverb
 *
 *
 * This file does -not- generate an object file.
 * Instead, it is #included in several places in fluid_voice.c.
 * The motivation for this is
 * - Calling it as a subroutine may be time consuming, especially with optimization off
 * - The previous implementation as a macro was clumsy to handle
 *
 *
 * Fluid_voice.c sets a couple of variables before #including this:
 * - dsp_data: Pointer to the original waveform data
 * - dsp_left_buf: The generated signal goes here, left channel
 * - dsp_right_buf: right channel
 * - dsp_reverb_buf: Send to reverb unit
 * - dsp_chorus_buf: Send to chorus unit
 * - dsp_start: Start processing at this output buffer index
 * - dsp_end: End processing just before this output buffer index
 * - dsp_a1: Coefficient for the filter
 * - dsp_a2: same
 * - dsp_b0: same
 * - dsp_b1: same
 * - dsp_b2: same
 * - dsp_filter_flag: Set, the filter is needed (many sound fonts don't use
 *                    the filter at all. If it is left at its default setting
 *                    of roughly 20 kHz, there is no need to apply filterling.)
 * - dsp_interp_method: Which interpolation method to use.
 * - voice holds the voice structure
 *
 * Some variables are set and modified:
 * - dsp_phase: The position in the original waveform data.
 *              This has an integer and a fractional part (between samples).
 * - dsp_phase_incr: For each output sample, the position in the original
 *              waveform advances by dsp_phase_incr. This also has an integer
 *              part and a fractional part.
 *              If a sample is played at root pitch (no pitch change),
 *              dsp_phase_incr is integer=1 and fractional=0.
 * - dsp_amp: The current amplitude envelope value.
 * - dsp_amp_incr: The changing rate of the amplitude envelope.
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_i: Index through the output buffer
 * - dsp_phase_fractional: The fractional part of dsp_phase
 * - dsp_coeff: A table of four coefficients, depending on the fractional phase.
 *              Used to interpolate between samples.
 * - dsp_process_buffer: Holds the processed signal between stages
 * - dsp_centernode: delay line for the IIR filter
 * - dsp_hist1: same
 * - dsp_hist2: same
 *
 */


/* Nonoptimized DSP loop */
#warning "This code is meant for experiments only.";

/* wave table interpolation */
for (dsp_i = dsp_start; dsp_i < dsp_end; dsp_i++) {

	dsp_coeff = &interp_coeff[fluid_phase_fract_to_tablerow(dsp_phase)];
	dsp_phase_index = fluid_phase_index(dsp_phase);
	dsp_sample = (dsp_amp *
		      (dsp_coeff->a0 * dsp_data[dsp_phase_index]
		       + dsp_coeff->a1 * dsp_data[dsp_phase_index+1]
		       + dsp_coeff->a2 * dsp_data[dsp_phase_index+2]
		       + dsp_coeff->a3 * dsp_data[dsp_phase_index+3]));

	/* increment phase and amplitude */
	fluid_phase_incr(dsp_phase, dsp_phase_incr);
	dsp_amp += dsp_amp_incr;

	/* filter */
	/* The filter is implemented in Direct-II form. */
	dsp_centernode = dsp_sample - dsp_a1 * dsp_hist1 - dsp_a2 * dsp_hist2;
	dsp_sample = dsp_b0 * dsp_centernode + dsp_b1 * dsp_hist1 + dsp_b2 * dsp_hist2;
	dsp_hist2 = dsp_hist1;
	dsp_hist1 = dsp_centernode;

	/* pan */
	dsp_left_buf[dsp_i] += voice->amp_left * dsp_sample;
	dsp_right_buf[dsp_i] += voice->amp_right * dsp_sample;

	/* reverb */
	if (dsp_reverb_buf){
		dsp_reverb_buf[dsp_i] += voice->amp_reverb * dsp_sample;
	}

	/* chorus */
	if (dsp_chorus_buf){
		dsp_chorus_buf[dsp_i] += voice->amp_chorus * dsp_sample;
	}
}

