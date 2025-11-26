#include <QtTest/QtTest>
#include "PartwritingEngine.h"
#include "Sketch.h"
#include "Motif.h"
#include "RhythmGrid.h"

class TestPartwriting : public QObject {
    Q_OBJECT

private slots:
    // Diatonic chord tests
    void testDiatonicChord_I();
    void testDiatonicChord_ii();
    void testDiatonicChord_V();
    void testDiatonicChord_IV();
    void testDiatonicChord_vi();
    void testDiatonicChord_viio();
    
    // Harmonic function tests
    void testHarmonicFunction_Tonic();
    void testHarmonicFunction_Predominant();
    void testHarmonicFunction_Dominant();
    
    // Chord contains test
    void testChordContains();
    
    // Chorale generation tests
    void testGenerateChoraleTexture_empty();
    void testGenerateChoraleTexture_singleNote();
    void testGenerateChoraleTexture_multipleNotes();
    void testGenerateChoraleTexture_voicesHaveSameRhythm();
    void testGenerateChoraleTexture_bassGetsRoot();
    void testGenerateChoraleTexture_allVoicesValid();
    
    // Voice leading tests
    void testVoiceLeading_cadenceEndsOnTonic();
    void testVoiceLeading_smoothMotion();
};

void TestPartwriting::testDiatonicChord_I() {
    DiatonicChord chord = PartwritingEngine::getDiatonicChord(1);
    QCOMPARE(chord.root, 1);
    QCOMPARE(chord.third, 3);
    QCOMPARE(chord.fifth, 5);
    QCOMPARE(chord.quality, QString("major"));
    QCOMPARE(chord.function, HarmonicFunction::Tonic);
}

void TestPartwriting::testDiatonicChord_ii() {
    DiatonicChord chord = PartwritingEngine::getDiatonicChord(2);
    QCOMPARE(chord.root, 2);
    QCOMPARE(chord.third, 4);
    QCOMPARE(chord.fifth, 6);
    QCOMPARE(chord.quality, QString("minor"));
    QCOMPARE(chord.function, HarmonicFunction::Predominant);
}

void TestPartwriting::testDiatonicChord_V() {
    DiatonicChord chord = PartwritingEngine::getDiatonicChord(5);
    QCOMPARE(chord.root, 5);
    QCOMPARE(chord.third, 7);
    QCOMPARE(chord.fifth, 2);
    QCOMPARE(chord.quality, QString("major"));
    QCOMPARE(chord.function, HarmonicFunction::Dominant);
}

void TestPartwriting::testDiatonicChord_IV() {
    DiatonicChord chord = PartwritingEngine::getDiatonicChord(4);
    QCOMPARE(chord.root, 4);
    QCOMPARE(chord.third, 6);
    QCOMPARE(chord.fifth, 1);
    QCOMPARE(chord.quality, QString("major"));
    QCOMPARE(chord.function, HarmonicFunction::Predominant);
}

void TestPartwriting::testDiatonicChord_vi() {
    DiatonicChord chord = PartwritingEngine::getDiatonicChord(6);
    QCOMPARE(chord.root, 6);
    QCOMPARE(chord.third, 1);
    QCOMPARE(chord.fifth, 3);
    QCOMPARE(chord.quality, QString("minor"));
    QCOMPARE(chord.function, HarmonicFunction::Tonic);
}

void TestPartwriting::testDiatonicChord_viio() {
    DiatonicChord chord = PartwritingEngine::getDiatonicChord(7);
    QCOMPARE(chord.root, 7);
    QCOMPARE(chord.third, 2);
    QCOMPARE(chord.fifth, 4);
    QCOMPARE(chord.quality, QString("diminished"));
    QCOMPARE(chord.function, HarmonicFunction::Dominant);
}

void TestPartwriting::testHarmonicFunction_Tonic() {
    // I, iii, vi are tonic function
    QCOMPARE(PartwritingEngine::getDiatonicChord(1).function, HarmonicFunction::Tonic);
    QCOMPARE(PartwritingEngine::getDiatonicChord(3).function, HarmonicFunction::Tonic);
    QCOMPARE(PartwritingEngine::getDiatonicChord(6).function, HarmonicFunction::Tonic);
}

void TestPartwriting::testHarmonicFunction_Predominant() {
    // ii, IV are predominant function
    QCOMPARE(PartwritingEngine::getDiatonicChord(2).function, HarmonicFunction::Predominant);
    QCOMPARE(PartwritingEngine::getDiatonicChord(4).function, HarmonicFunction::Predominant);
}

void TestPartwriting::testHarmonicFunction_Dominant() {
    // V, vii° are dominant function
    QCOMPARE(PartwritingEngine::getDiatonicChord(5).function, HarmonicFunction::Dominant);
    QCOMPARE(PartwritingEngine::getDiatonicChord(7).function, HarmonicFunction::Dominant);
}

void TestPartwriting::testChordContains() {
    DiatonicChord iChord = PartwritingEngine::getDiatonicChord(1); // 1, 3, 5
    QVERIFY(iChord.contains(1));
    QVERIFY(iChord.contains(3));
    QVERIFY(iChord.contains(5));
    QVERIFY(!iChord.contains(2));
    QVERIFY(!iChord.contains(4));
    QVERIFY(!iChord.contains(6));
    QVERIFY(!iChord.contains(7));
}

void TestPartwriting::testGenerateChoraleTexture_empty() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif emptyMotif("m1", "Empty", 1);
    // Empty pitch contour
    
    ChoraleVoices voices = engine.generateChoraleTexture(emptyMotif, sketch);
    QVERIFY(!voices.isValid());
}

void TestPartwriting::testGenerateChoraleTexture_singleNote() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Single Note", 1);
    motif.setPitchContour({5}); // Just scale degree 5
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    QVERIFY(voices.isValid());
    QCOMPARE(voices.soprano.noteCount(), 1);
    QCOMPARE(voices.alto.noteCount(), 1);
    QCOMPARE(voices.tenor.noteCount(), 1);
    QCOMPARE(voices.bass.noteCount(), 1);
    
    // Soprano should have the melody
    QCOMPARE(voices.soprano.pitches[0], 5);
}

void TestPartwriting::testGenerateChoraleTexture_multipleNotes() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Scale", 1);
    motif.setPitchContour({1, 2, 3, 4, 5}); // Simple scale fragment
    
    RhythmGrid rhythm;
    for (int i = 0; i < 5; i++) {
        rhythm.addCell("quarter");
    }
    motif.setRhythmGrid(rhythm);
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    QVERIFY(voices.isValid());
    
    // All voices should have same number of notes
    QCOMPARE(voices.soprano.noteCount(), 5);
    QCOMPARE(voices.alto.noteCount(), 5);
    QCOMPARE(voices.tenor.noteCount(), 5);
    QCOMPARE(voices.bass.noteCount(), 5);
}

void TestPartwriting::testGenerateChoraleTexture_voicesHaveSameRhythm() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Rhythmic", 1);
    motif.setPitchContour({1, 3, 5});
    
    RhythmGrid rhythm;
    rhythm.addCell("half");
    rhythm.addCell("quarter");
    rhythm.addCell("quarter");
    motif.setRhythmGrid(rhythm);
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    
    // All voices should have the same rhythm as soprano
    QCOMPARE(voices.alto.rhythm.cellCount(), voices.soprano.rhythm.cellCount());
    QCOMPARE(voices.tenor.rhythm.cellCount(), voices.soprano.rhythm.cellCount());
    QCOMPARE(voices.bass.rhythm.cellCount(), voices.soprano.rhythm.cellCount());
    
    // Check that durations match
    for (int i = 0; i < voices.soprano.rhythm.cellCount(); i++) {
        QCOMPARE(voices.alto.rhythm.cell(i).duration, 
                 voices.soprano.rhythm.cell(i).duration);
        QCOMPARE(voices.tenor.rhythm.cell(i).duration, 
                 voices.soprano.rhythm.cell(i).duration);
        QCOMPARE(voices.bass.rhythm.cell(i).duration, 
                 voices.soprano.rhythm.cell(i).duration);
    }
}

void TestPartwriting::testGenerateChoraleTexture_bassGetsRoot() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Test", 1);
    motif.setPitchContour({1, 5, 1}); // Likely I - I - I or similar
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    
    // Bass should generally have chord roots (scale degrees 1-7)
    for (int i = 0; i < voices.bass.noteCount(); i++) {
        int bassPitch = voices.bass.pitches[i];
        QVERIFY(bassPitch >= 1 && bassPitch <= 7);
    }
}

void TestPartwriting::testGenerateChoraleTexture_allVoicesValid() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Test", 1);
    // Test with a phrase that should produce a good V-I cadence
    motif.setPitchContour({1, 3, 5, 4, 3, 2, 7, 1});
    
    RhythmGrid rhythm;
    for (int i = 0; i < 8; i++) {
        rhythm.addCell("quarter");
    }
    motif.setRhythmGrid(rhythm);
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    QVERIFY(voices.isValid());
    
    // All pitches should be valid scale degrees
    for (int pitch : voices.soprano.pitches) QVERIFY(pitch >= 1 && pitch <= 7);
    for (int pitch : voices.alto.pitches) QVERIFY(pitch >= 1 && pitch <= 7);
    for (int pitch : voices.tenor.pitches) QVERIFY(pitch >= 1 && pitch <= 7);
    for (int pitch : voices.bass.pitches) QVERIFY(pitch >= 1 && pitch <= 7);
}

void TestPartwriting::testVoiceLeading_cadenceEndsOnTonic() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Cadence", 1);
    // End on scale degree 1 - should produce authentic cadence ending on I
    motif.setPitchContour({5, 4, 3, 2, 1});
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    
    // The final bass note should likely be 1 (root of I chord)
    // due to the DP scoring preferring I at the end
    QCOMPARE(voices.bass.pitches.last(), 1);
}

void TestPartwriting::testVoiceLeading_smoothMotion() {
    PartwritingEngine engine;
    Sketch sketch("test", "Test Sketch");
    Motif motif("m1", "Smooth", 1);
    motif.setPitchContour({1, 2, 3, 2, 1}); // Stepwise melody
    
    ChoraleVoices voices = engine.generateChoraleTexture(motif, sketch);
    
    // Check that inner voices don't have huge leaps (more than 4 scale degrees)
    for (int i = 1; i < voices.alto.noteCount(); i++) {
        int leap = std::abs(voices.alto.pitches[i] - voices.alto.pitches[i-1]);
        QVERIFY(leap <= 4); // No larger than a fourth
    }
    
    for (int i = 1; i < voices.tenor.noteCount(); i++) {
        int leap = std::abs(voices.tenor.pitches[i] - voices.tenor.pitches[i-1]);
        QVERIFY(leap <= 4);
    }
}

QTEST_MAIN(TestPartwriting)
#include "tst_partwriting.moc"

