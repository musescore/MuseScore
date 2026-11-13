# Export a score selection as rehearsal audio

## What the command does

**Export selection…** creates an audio file from a contiguous range in the
current score. It renders the original score directly, so markings and playback
context established before the selection—such as tempo, dynamics,
articulations, playing techniques, instrument changes, effects, and the current
sound profile—continue to affect the exported fragment.

The command does not create a second score and does not change or dirty the
open project.

## Select and export

1. Make a range selection in the score. Its horizontal extent determines the
   exported time range.
2. Include one or more staves in the vertical extent to choose the instruments
   to emphasize.
3. Right-click the selection and choose **Export selection…**, or use the same
   command in the File menu.
4. Adjust the rehearsal settings and select **Export…**.

## Rehearsal settings

- **Tempo** changes playback speed from 10% to 300% of the score tempo without
  changing the score.
- **Metronome** adds the score's click throughout the exported fragment. When
  enabled, fade-in is unavailable so the first click is not attenuated.
- **Fade in** includes audio immediately before the selection and fades it in
  over the requested duration. The margin is shortened safely at the beginning
  of a score.
- **Fade out** includes audio immediately after the selection and fades it out
  over the requested duration. The margin is shortened safely at the end of a
  score.
- **Mute other instruments** removes instruments outside the vertical
  selection.
- **Other instruments** applies one background level to all instruments outside
  the vertical selection. Expanding **Individual other-instrument levels**
  allows exceptions.
- **Selected instrument levels** adjusts each focused instrument relative to
  its existing mixer level.

With one selected instrument, the other instruments start muted. With two or
more selected instruments, they start at 50%. Focused instruments start at
100%. Every level is a temporary multiplier on the existing mix.

## Formats

The dialog reuses MuseScore's existing audio encoders and exposes only formats
available in the current build. Format-specific controls such as sample rate,
sample format, and bit rate retain their normal meaning.

## Current scope

- The selection must be a contiguous range.
- Instrument focus is resolved by selected parts/staves, not by individual
  voices within a staff.
- The metronome is either off or active throughout the fragment. A dedicated
  one-measure count-in is reserved for a future version.
