#include "Motif.h"
#include "RhythmGrid.h"
#include "Section.h"
#include "Sketch.h"
#include <QTest>
#include <QUuid>

class TestSection : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Section basic tests
  void testSectionCreation();
  void testAddPlacement();
  void testRemovePlacement();
  void testMovePlacement();
  void testSetRepetitions();
  void testSortedPlacements();

  // Timeline flattening tests
  void testFlattenEmptySection();
  void testFlattenSingleMotif();
  void testFlattenMultipleMotifs();
  void testFlattenWithRepetitions();
  void testFlattenWithRests();

  // Serialization tests
  void testPlacementSerialization();
  void testSectionSerialization();
};

void TestSection::initTestCase() {}

void TestSection::cleanupTestCase() {}

void TestSection::testSectionCreation() {
  Section section("sec-1", "Verse", 8);

  QCOMPARE(section.id(), QString("sec-1"));
  QCOMPARE(section.name(), QString("Verse"));
  QCOMPARE(section.lengthBars(), 8);
  QCOMPARE(section.placements().size(), 0);
}

void TestSection::testAddPlacement() {
  Section section("sec-1", "Verse", 8);

  MotifPlacement p1;
  p1.motifId = "motif-1";
  p1.startBar = 0;
  p1.repetitions = 1;
  p1.voice = 0;

  section.addPlacement(p1);
  QCOMPARE(section.placements().size(), 1);

  MotifPlacement p2;
  p2.motifId = "motif-2";
  p2.startBar = 4;
  p2.repetitions = 2;
  p2.voice = 0;

  section.addPlacement(p2);
  QCOMPARE(section.placements().size(), 2);

  QCOMPARE(section.placements()[0].motifId, QString("motif-1"));
  QCOMPARE(section.placements()[1].motifId, QString("motif-2"));
  QCOMPARE(section.placements()[1].repetitions, 2);
}

void TestSection::testRemovePlacement() {
  Section section("sec-1", "Verse", 8);

  MotifPlacement p1;
  p1.motifId = "motif-1";
  p1.startBar = 0;
  section.addPlacement(p1);

  MotifPlacement p2;
  p2.motifId = "motif-2";
  p2.startBar = 2;
  section.addPlacement(p2);

  MotifPlacement p3;
  p3.motifId = "motif-3";
  p3.startBar = 4;
  section.addPlacement(p3);

  QCOMPARE(section.placements().size(), 3);

  // Remove middle placement
  section.removePlacement(1);
  QCOMPARE(section.placements().size(), 2);
  QCOMPARE(section.placements()[0].motifId, QString("motif-1"));
  QCOMPARE(section.placements()[1].motifId, QString("motif-3"));

  // Remove by motif ID
  section.removePlacementByMotifId("motif-1");
  QCOMPARE(section.placements().size(), 1);
  QCOMPARE(section.placements()[0].motifId, QString("motif-3"));
}

void TestSection::testMovePlacement() {
  Section section("sec-1", "Verse", 8);

  MotifPlacement p;
  p.motifId = "motif-1";
  p.startBar = 0;
  section.addPlacement(p);

  QCOMPARE(section.placements()[0].startBar, 0);

  section.movePlacement(0, 4);
  QCOMPARE(section.placements()[0].startBar, 4);

  section.movePlacement(0, 2);
  QCOMPARE(section.placements()[0].startBar, 2);
}

void TestSection::testSetRepetitions() {
  Section section("sec-1", "Verse", 8);

  MotifPlacement p;
  p.motifId = "motif-1";
  p.startBar = 0;
  p.repetitions = 1;
  section.addPlacement(p);

  QCOMPARE(section.placements()[0].repetitions, 1);

  section.setPlacementRepetitions(0, 3);
  QCOMPARE(section.placements()[0].repetitions, 3);

  // Test endBar calculation
  int motifLength = 2; // 2 bars
  QCOMPARE(section.placements()[0].endBar(motifLength), 6); // 0 + 2*3 = 6
}

void TestSection::testSortedPlacements() {
  Section section("sec-1", "Verse", 8);

  MotifPlacement p1;
  p1.motifId = "motif-1";
  p1.startBar = 4;
  section.addPlacement(p1);

  MotifPlacement p2;
  p2.motifId = "motif-2";
  p2.startBar = 0;
  section.addPlacement(p2);

  MotifPlacement p3;
  p3.motifId = "motif-3";
  p3.startBar = 2;
  section.addPlacement(p3);

  QList<MotifPlacement> sorted = section.sortedPlacements();

  QCOMPARE(sorted[0].motifId, QString("motif-2")); // startBar 0
  QCOMPARE(sorted[1].motifId, QString("motif-3")); // startBar 2
  QCOMPARE(sorted[2].motifId, QString("motif-1")); // startBar 4
}

void TestSection::testFlattenEmptySection() {
  Sketch sketch("sketch-1", "Test Sketch");
  Section section("sec-1", "Empty", 4);

  QList<NoteEvent> timeline = section.flattenToTimeline(sketch);
  QCOMPARE(timeline.size(), 0);
}

void TestSection::testFlattenSingleMotif() {
  Sketch sketch("sketch-1", "Test Sketch");

  // Create a motif with 3 notes
  Motif motif("motif-1", "Test Motif", 1);
  motif.setPitchContour({1, 3, 5});

  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"quarter", false, false});
  grid.addCell({"half", false, false});
  motif.setRhythmGrid(grid);

  sketch.addMotif(motif);

  Section section("sec-1", "Verse", 4);
  MotifPlacement p;
  p.motifId = "motif-1";
  p.startBar = 0;
  p.repetitions = 1;
  p.voice = 0;
  section.addPlacement(p);

  QList<NoteEvent> timeline = section.flattenToTimeline(sketch);

  QCOMPARE(timeline.size(), 3);
  QCOMPARE(timeline[0].scaleDegree, 1);
  QCOMPARE(timeline[0].duration, QString("quarter"));
  QCOMPARE(timeline[0].startBeat, 0.0);

  QCOMPARE(timeline[1].scaleDegree, 3);
  QCOMPARE(timeline[1].startBeat, 1.0);

  QCOMPARE(timeline[2].scaleDegree, 5);
  QCOMPARE(timeline[2].duration, QString("half"));
  QCOMPARE(timeline[2].startBeat, 2.0);
}

void TestSection::testFlattenMultipleMotifs() {
  Sketch sketch("sketch-1", "Test Sketch");

  // Motif 1: 2 notes, 1 bar
  Motif motif1("motif-1", "Motif A", 1);
  motif1.setPitchContour({1, 2});
  RhythmGrid grid1;
  grid1.addCell({"half", false, false});
  grid1.addCell({"half", false, false});
  motif1.setRhythmGrid(grid1);
  sketch.addMotif(motif1);

  // Motif 2: 2 notes, 1 bar
  Motif motif2("motif-2", "Motif B", 1);
  motif2.setPitchContour({5, 6});
  RhythmGrid grid2;
  grid2.addCell({"half", false, false});
  grid2.addCell({"half", false, false});
  motif2.setRhythmGrid(grid2);
  sketch.addMotif(motif2);

  Section section("sec-1", "Verse", 4);

  MotifPlacement p1;
  p1.motifId = "motif-1";
  p1.startBar = 0;
  p1.repetitions = 1;
  section.addPlacement(p1);

  MotifPlacement p2;
  p2.motifId = "motif-2";
  p2.startBar = 2; // Start at bar 3
  p2.repetitions = 1;
  section.addPlacement(p2);

  QList<NoteEvent> timeline = section.flattenToTimeline(sketch);

  QCOMPARE(timeline.size(), 4);

  // First motif notes at beat 0, 2
  QCOMPARE(timeline[0].scaleDegree, 1);
  QCOMPARE(timeline[0].startBeat, 0.0);
  QCOMPARE(timeline[1].scaleDegree, 2);
  QCOMPARE(timeline[1].startBeat, 2.0);

  // Second motif notes at beat 8, 10 (bar 2 = beat 8)
  QCOMPARE(timeline[2].scaleDegree, 5);
  QCOMPARE(timeline[2].startBeat, 8.0);
  QCOMPARE(timeline[3].scaleDegree, 6);
  QCOMPARE(timeline[3].startBeat, 10.0);
}

void TestSection::testFlattenWithRepetitions() {
  Sketch sketch("sketch-1", "Test Sketch");

  // 1-bar motif with 2 quarter notes
  Motif motif("motif-1", "Short", 1);
  motif.setPitchContour({1, 2});
  RhythmGrid grid;
  grid.addCell({"half", false, false});
  grid.addCell({"half", false, false});
  motif.setRhythmGrid(grid);
  sketch.addMotif(motif);

  Section section("sec-1", "Verse", 4);

  MotifPlacement p;
  p.motifId = "motif-1";
  p.startBar = 0;
  p.repetitions = 2; // Repeat twice
  section.addPlacement(p);

  QList<NoteEvent> timeline = section.flattenToTimeline(sketch);

  // 2 notes × 2 repetitions = 4 notes
  QCOMPARE(timeline.size(), 4);

  // First repetition
  QCOMPARE(timeline[0].scaleDegree, 1);
  QCOMPARE(timeline[0].startBeat, 0.0);
  QCOMPARE(timeline[1].scaleDegree, 2);
  QCOMPARE(timeline[1].startBeat, 2.0);

  // Second repetition (starts at bar 1 = beat 4)
  QCOMPARE(timeline[2].scaleDegree, 1);
  QCOMPARE(timeline[2].startBeat, 4.0);
  QCOMPARE(timeline[3].scaleDegree, 2);
  QCOMPARE(timeline[3].startBeat, 6.0);
}

void TestSection::testFlattenWithRests() {
  Sketch sketch("sketch-1", "Test Sketch");

  // Motif with note, rest, note
  Motif motif("motif-1", "With Rests", 1);
  motif.setPitchContour({1, 5}); // Only 2 pitches for 2 notes

  RhythmGrid grid;
  grid.addCell({"quarter", false, false}); // Note
  grid.addCell({"quarter", false, true});  // Rest
  grid.addCell({"half", false, false});    // Note
  motif.setRhythmGrid(grid);
  sketch.addMotif(motif);

  Section section("sec-1", "Verse", 4);

  MotifPlacement p;
  p.motifId = "motif-1";
  p.startBar = 0;
  p.repetitions = 1;
  section.addPlacement(p);

  QList<NoteEvent> timeline = section.flattenToTimeline(sketch);

  QCOMPARE(timeline.size(), 3);

  QCOMPARE(timeline[0].scaleDegree, 1);
  QCOMPARE(timeline[0].isRest, false);
  QCOMPARE(timeline[0].startBeat, 0.0);

  QCOMPARE(timeline[1].isRest, true);
  QCOMPARE(timeline[1].scaleDegree, 0); // Rests have no pitch
  QCOMPARE(timeline[1].startBeat, 1.0);

  QCOMPARE(timeline[2].scaleDegree, 5);
  QCOMPARE(timeline[2].isRest, false);
  QCOMPARE(timeline[2].startBeat, 2.0);
}

void TestSection::testPlacementSerialization() {
  MotifPlacement original;
  original.motifId = "motif-123";
  original.startBar = 4;
  original.repetitions = 3;
  original.voice = 1;

  QJsonObject json = original.toJson();

  MotifPlacement restored = MotifPlacement::fromJson(json);

  QCOMPARE(restored.motifId, QString("motif-123"));
  QCOMPARE(restored.startBar, 4);
  QCOMPARE(restored.repetitions, 3);
  QCOMPARE(restored.voice, 1);
}

void TestSection::testSectionSerialization() {
  Section original("sec-1", "Chorus", 16);

  MotifPlacement p1;
  p1.motifId = "motif-1";
  p1.startBar = 0;
  p1.repetitions = 2;
  original.addPlacement(p1);

  MotifPlacement p2;
  p2.motifId = "motif-2";
  p2.startBar = 8;
  p2.repetitions = 1;
  original.addPlacement(p2);

  QJsonObject json = original.toJson();

  Section restored = Section::fromJson(json);

  QCOMPARE(restored.id(), QString("sec-1"));
  QCOMPARE(restored.name(), QString("Chorus"));
  QCOMPARE(restored.lengthBars(), 16);
  QCOMPARE(restored.placements().size(), 2);

  QCOMPARE(restored.placements()[0].motifId, QString("motif-1"));
  QCOMPARE(restored.placements()[0].repetitions, 2);
  QCOMPARE(restored.placements()[1].startBar, 8);
}

QTEST_MAIN(TestSection)
#include "tst_section.moc"

