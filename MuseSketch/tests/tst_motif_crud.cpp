#include "Motif.h"
#include "RhythmGrid.h"
#include "Sketch.h"
#include <QTest>
#include <QUuid>

class TestMotifCrud : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Sketch motif management tests
  void testAddMotif();
  void testRemoveMotif();
  void testFindMotif();

  // Motif property tests
  void testMotifRename();
  void testMotifDuplicate();
  void testMotifPitchUpdate();
  void testMotifRhythmUpdate();

  // Serialization round-trip
  void testMotifSerialization();
  void testSketchWithMotifsSerialization();

  // Edge cases
  void testRemoveNonexistentMotif();
  void testFindNonexistentMotif();
  void testDuplicateWithRhythmData();
};

void TestMotifCrud::initTestCase() {}

void TestMotifCrud::cleanupTestCase() {}

// Test adding motifs to a sketch
void TestMotifCrud::testAddMotif() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  QCOMPARE(sketch.motifs().size(), 0);

  Motif motif1("motif-1", "First Motif", 2);
  sketch.addMotif(motif1);

  QCOMPARE(sketch.motifs().size(), 1);
  QCOMPARE(sketch.motifs()[0].name(), QString("First Motif"));
  QCOMPARE(sketch.motifs()[0].lengthBars(), 2);

  Motif motif2("motif-2", "Second Motif", 4);
  sketch.addMotif(motif2);

  QCOMPARE(sketch.motifs().size(), 2);
  QCOMPARE(sketch.motifs()[1].name(), QString("Second Motif"));
}

// Test removing motifs from a sketch
void TestMotifCrud::testRemoveMotif() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  Motif motif1("motif-1", "First Motif", 1);
  Motif motif2("motif-2", "Second Motif", 1);
  Motif motif3("motif-3", "Third Motif", 1);

  sketch.addMotif(motif1);
  sketch.addMotif(motif2);
  sketch.addMotif(motif3);

  QCOMPARE(sketch.motifs().size(), 3);

  // Remove middle motif
  sketch.removeMotif("motif-2");
  QCOMPARE(sketch.motifs().size(), 2);

  // Verify correct motifs remain
  QCOMPARE(sketch.motifs()[0].id(), QString("motif-1"));
  QCOMPARE(sketch.motifs()[1].id(), QString("motif-3"));

  // Remove first motif
  sketch.removeMotif("motif-1");
  QCOMPARE(sketch.motifs().size(), 1);
  QCOMPARE(sketch.motifs()[0].id(), QString("motif-3"));

  // Remove last motif
  sketch.removeMotif("motif-3");
  QCOMPARE(sketch.motifs().size(), 0);
}

// Test finding motifs by ID
void TestMotifCrud::testFindMotif() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  Motif motif1("motif-1", "First Motif", 1);
  motif1.setPitchContour({1, 3, 5});

  Motif motif2("motif-2", "Second Motif", 2);
  motif2.setPitchContour({2, 4, 6, 7});

  sketch.addMotif(motif1);
  sketch.addMotif(motif2);

  // Find existing motif
  Motif *found = sketch.findMotif("motif-1");
  QVERIFY(found != nullptr);
  QCOMPARE(found->name(), QString("First Motif"));
  QCOMPARE(found->pitchContour().size(), 3);

  // Find second motif
  Motif *found2 = sketch.findMotif("motif-2");
  QVERIFY(found2 != nullptr);
  QCOMPARE(found2->name(), QString("Second Motif"));
  QCOMPARE(found2->lengthBars(), 2);
}

// Test renaming a motif
void TestMotifCrud::testMotifRename() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  Motif motif("motif-1", "Original Name", 1);
  sketch.addMotif(motif);

  // Find and rename
  Motif *found = sketch.findMotif("motif-1");
  QVERIFY(found != nullptr);
  QCOMPARE(found->name(), QString("Original Name"));

  found->setName("Renamed Motif");
  QCOMPARE(found->name(), QString("Renamed Motif"));

  // Verify the change persists in the sketch
  Motif *foundAgain = sketch.findMotif("motif-1");
  QCOMPARE(foundAgain->name(), QString("Renamed Motif"));
}

// Test duplicating a motif
void TestMotifCrud::testMotifDuplicate() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  // Create original motif with data
  Motif original("motif-1", "Original Motif", 2);
  original.setPitchContour({1, 2, 3, 4, 5});
  original.setKeyRef("C major");

  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", false, false});
  grid.addCell({"eighth", true, false}); // Tied
  grid.addCell({"half", false, true});   // Rest
  grid.addCell({"quarter", false, false});
  original.setRhythmGrid(grid);

  sketch.addMotif(original);

  // Create duplicate
  QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
  Motif duplicate(newId, original.name() + " (copy)", original.lengthBars());
  duplicate.setPitchContour(original.pitchContour());
  duplicate.setRhythmGrid(original.rhythmGrid());
  duplicate.setKeyRef(original.keyRef());

  sketch.addMotif(duplicate);

  QCOMPARE(sketch.motifs().size(), 2);

  // Verify duplicate has same data but different ID
  Motif *dup = sketch.findMotif(newId);
  QVERIFY(dup != nullptr);
  QCOMPARE(dup->name(), QString("Original Motif (copy)"));
  QCOMPARE(dup->lengthBars(), 2);
  QCOMPARE(dup->pitchContour(), original.pitchContour());
  QCOMPARE(dup->rhythmGrid().cellCount(), original.rhythmGrid().cellCount());
  QCOMPARE(dup->keyRef(), QString("C major"));

  // Verify original is unchanged
  Motif *orig = sketch.findMotif("motif-1");
  QCOMPARE(orig->name(), QString("Original Motif"));
}

// Test updating pitch contour
void TestMotifCrud::testMotifPitchUpdate() {
  Motif motif("motif-1", "Test Motif", 1);
  motif.setPitchContour({1, 3, 5});

  QCOMPARE(motif.pitchContour().size(), 3);
  QCOMPARE(motif.pitchContour()[0], 1);
  QCOMPARE(motif.pitchContour()[1], 3);
  QCOMPARE(motif.pitchContour()[2], 5);

  // Update pitches
  motif.setPitchContour({7, 6, 5, 4, 3, 2, 1});

  QCOMPARE(motif.pitchContour().size(), 7);
  QCOMPARE(motif.pitchContour()[0], 7);
  QCOMPARE(motif.pitchContour()[6], 1);
}

// Test updating rhythm data
void TestMotifCrud::testMotifRhythmUpdate() {
  Motif motif("motif-1", "Test Motif", 1);

  QVERIFY(motif.rhythmGrid().isEmpty());

  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", false, false});
  grid.addCell({"eighth", false, true}); // Rest

  motif.setRhythmGrid(grid);

  QCOMPARE(motif.rhythmGrid().cellCount(), 3);
  QCOMPARE(motif.rhythmGrid().cell(0).duration, QString("quarter"));
  QCOMPARE(motif.rhythmGrid().cell(2).isRest, true);

  // Test legacy rhythmPattern accessor
  QList<QString> pattern = motif.rhythmPattern();
  QCOMPARE(pattern.size(), 3);
  QCOMPARE(pattern[0], QString("quarter"));
}

// Test motif JSON serialization
void TestMotifCrud::testMotifSerialization() {
  Motif original("motif-1", "Serialization Test", 3);
  original.setPitchContour({1, 2, 3, 4, 5});
  original.setKeyRef("G major");

  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", true, false}); // Tied
  grid.addCell({"half", false, true});   // Rest
  original.setRhythmGrid(grid);

  // Serialize
  QJsonObject json = original.toJson();

  // Deserialize
  Motif restored = Motif::fromJson(json);

  QCOMPARE(restored.id(), QString("motif-1"));
  QCOMPARE(restored.name(), QString("Serialization Test"));
  QCOMPARE(restored.lengthBars(), 3);
  QCOMPARE(restored.keyRef(), QString("G major"));
  QCOMPARE(restored.pitchContour(), original.pitchContour());

  QCOMPARE(restored.rhythmGrid().cellCount(), 3);
  QCOMPARE(restored.rhythmGrid().cell(0).duration, QString("quarter"));
  QCOMPARE(restored.rhythmGrid().cell(1).tie, true);
  QCOMPARE(restored.rhythmGrid().cell(2).isRest, true);
}

// Test sketch with motifs serialization
void TestMotifCrud::testSketchWithMotifsSerialization() {
  Sketch original("sketch-1", "Full Sketch");
  original.setKey("D minor");
  original.setTempo(140);
  original.setTimeSignature("3/4");

  Motif motif1("m1", "Motif One", 1);
  motif1.setPitchContour({1, 3, 5, 3, 1});

  Motif motif2("m2", "Motif Two", 2);
  motif2.setPitchContour({5, 4, 3, 2, 1});

  original.addMotif(motif1);
  original.addMotif(motif2);

  // Serialize
  QJsonObject json = original.toJson();

  // Deserialize
  Sketch restored = Sketch::fromJson(json);

  QCOMPARE(restored.id(), QString("sketch-1"));
  QCOMPARE(restored.name(), QString("Full Sketch"));
  QCOMPARE(restored.key(), QString("D minor"));
  QCOMPARE(restored.tempo(), 140);
  QCOMPARE(restored.timeSignature(), QString("3/4"));

  QCOMPARE(restored.motifs().size(), 2);
  QCOMPARE(restored.motifs()[0].name(), QString("Motif One"));
  QCOMPARE(restored.motifs()[1].name(), QString("Motif Two"));
  QCOMPARE(restored.motifs()[0].pitchContour().size(), 5);
}

// Test removing a motif that doesn't exist
void TestMotifCrud::testRemoveNonexistentMotif() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  Motif motif("motif-1", "Only Motif", 1);
  sketch.addMotif(motif);

  QCOMPARE(sketch.motifs().size(), 1);

  // Try to remove non-existent motif
  sketch.removeMotif("nonexistent-id");

  // Original should still be there
  QCOMPARE(sketch.motifs().size(), 1);
  QCOMPARE(sketch.motifs()[0].id(), QString("motif-1"));
}

// Test finding a motif that doesn't exist
void TestMotifCrud::testFindNonexistentMotif() {
  Sketch sketch("test-sketch-1", "Test Sketch");

  Motif motif("motif-1", "Only Motif", 1);
  sketch.addMotif(motif);

  Motif *found = sketch.findMotif("nonexistent-id");
  QVERIFY(found == nullptr);
}

// Test duplicating motif with full rhythm data
void TestMotifCrud::testDuplicateWithRhythmData() {
  Motif original("motif-1", "Complex Motif", 4);
  original.setPitchContour({1, 2, 3, 4, 5, 6, 7, 1});

  RhythmGrid grid;
  grid.addCell({"whole", false, false});
  grid.addCell({"dotted-half", false, false});
  grid.addCell({"quarter", true, false}); // Tied
  grid.addCell({"half", false, true});    // Rest
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", false, false});
  grid.addCell({"eighth", false, false});
  grid.addCell({"quarter", false, false});
  original.setRhythmGrid(grid);

  // Duplicate
  Motif duplicate("motif-2", original.name() + " (copy)", original.lengthBars());
  duplicate.setPitchContour(original.pitchContour());
  duplicate.setRhythmGrid(original.rhythmGrid());

  // Verify all rhythm cells copied correctly
  QCOMPARE(duplicate.rhythmGrid().cellCount(), 8);
  QCOMPARE(duplicate.rhythmGrid().cell(0).duration, QString("whole"));
  QCOMPARE(duplicate.rhythmGrid().cell(1).duration, QString("dotted-half"));
  QCOMPARE(duplicate.rhythmGrid().cell(2).tie, true);
  QCOMPARE(duplicate.rhythmGrid().cell(3).isRest, true);

  // Verify total beats match
  QCOMPARE(duplicate.rhythmGrid().totalBeats(), original.rhythmGrid().totalBeats());
}

QTEST_MAIN(TestMotifCrud)
#include "tst_motif_crud.moc"

