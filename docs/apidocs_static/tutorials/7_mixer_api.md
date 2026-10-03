### Testing the plugin Mixer API

Create a QML plugin with a `MuseScore` root object and put this handler in its `onRun` block. Use a score with at least one part:

```qml
import QtQuick
import MuseScore 3.0

MuseScore {
    title: "Test Mixer API"

    onRun: {
    var score = curScore;
    var part = score.parts[0];
    var mixer = part.mixerChannel;

    var originalVolume = mixer.volume;
    var originalBalance = mixer.balance;
    var originalMuted = mixer.muted;
    var originalSolo = mixer.solo;
    var originalBank = mixer.midiBank;
    var originalProgram = mixer.midiProgram;

    mixer.volume = -6;
    mixer.balance = 0.25;
    mixer.muted = true;
    mixer.solo = true;

    if (mixer.volume !== -6 || mixer.balance !== 0.25 || !mixer.solo || mixer.muted) {
        throw new Error("Mixer controls did not update");
    }

    mixer.solo = originalSolo;
    mixer.muted = originalMuted;
    mixer.balance = originalBalance;
    mixer.volume = originalVolume;

    score.startCmd("Test mixer MIDI settings");
    mixer.midiBank = (originalBank + 1) % 256;
    mixer.midiProgram = (originalProgram + 1) % 128;
    score.endCmd();
    if (mixer.midiBank === originalBank || mixer.midiProgram === originalProgram) {
        throw new Error("MIDI settings did not update");
    }
    score.startCmd("Restore mixer MIDI settings");
    mixer.midiBank = originalBank;
    mixer.midiProgram = originalProgram;
    score.endCmd();

    mixer.availableSounds(function(sounds, error) {
        if (error) {
            throw new Error(error);
        }
        if (!sounds.length || !sounds[0].id || !sounds[0].name) {
            throw new Error("No usable sound resources were returned");
        }
        mixer.setSound(sounds[0].id, function(success, setSoundError) {
            if (setSoundError || !success) {
                throw new Error(setSoundError || "Sound selection failed");
            }
        });
    });
    }
}
```

## API reference

`Part.mixerChannel` exposes the primary instrument channel for a part. It is
`null` when the part has no instrument track.

| Member        | Type      | Behavior                                                                                         |
| ------------- | --------- | ------------------------------------------------------------------------------------------------ |
| `volume`      | `Number`  | Output level in decibels, clamped to `-60` through `12`.                                         |
| `balance`     | `Number`  | Stereo balance, clamped to `-1` (left) through `1` (right).                                      |
| `muted`       | `Boolean` | Enables or disables mute. Enabling mute clears `solo`.                                           |
| `solo`        | `Boolean` | Enables or disables solo. Enabling solo clears `muted` and may mute other channels in the Mixer. |
| `midiBank`    | `Number`  | MIDI bank, from `0` through `255`; changes are undoable.                                         |
| `midiProgram` | `Number`  | MIDI program, from `0` through `127`; changes are undoable.                                      |

The following methods are asynchronous:

```qml
mixer.availableSounds(function(sounds, error) { ... });
mixer.setSound(soundId, function(success, error) { ... });
```

`availableSounds` returns an array of objects with `id` and `name` fields. Pass
an `id` from that array to `setSound`. Both methods report failures through the
`error` callback argument; sound enumeration and selection do not block the
plugin handler.

Volume, balance, mute, solo, and sound changes are not undoable. MIDI bank and
program changes must be wrapped in `score.startCmd()` and `score.endCmd()` when
they should appear as undoable score operations.

Run the plugin on the score and verify each control changes in the Mixer, then returns to its original value. Sound enumeration and selection are asynchronous; verify the callback returns resource IDs and display names and that selecting an ID submits a source change. Use a disposable score for the sound-selection check.

MIDI bank and program changes are undoable and use the existing channel setter implementation.

The automated wrapper test is `Engraving_ApiScoreTests.partMixerChannelApi` in the `engraving_api_tests` target.
