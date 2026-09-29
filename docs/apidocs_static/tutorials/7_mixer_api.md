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

Run the plugin on the score and verify each control changes in the Mixer, then returns to its original value. Sound enumeration and selection are asynchronous; verify the callback returns resource IDs and display names and that selecting an ID submits a source change. Use a disposable score for the sound-selection check.

MIDI bank and program changes are undoable and use the existing channel setter implementation.

The automated wrapper test is `Engraving_ApiScoreTests.partMixerChannelApi` in the `engraving_api_tests` target.
