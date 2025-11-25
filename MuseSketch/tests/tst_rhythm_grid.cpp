#include "MotifEditorController.h"
#include "RhythmGrid.h"
#include <QSignalSpy>
#include <QTest>

class TestRhythmGrid : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // RhythmGrid data structure tests
  void testRhythmGridAddCell();
  void testRhythmGridRemoveCell();
  void testRhythmGridClear();
  void testRhythmGridCellAccess();
  void testRhythmGridSerialization();

  // RhythmCell tests
  void testRhythmCellSerialization();
  void testTotalBeats();

  // MotifEditorController rhythm interactions
  void testAddNote();
  void testAddRhythm();
  void testRemoveLastNote();
  void testToggleRest();
  void testToggleTie();
  void testClear();

  // Integration: contour to rhythm
  void testApplyContourPath();
};

void TestRhythmGrid::initTestCase() {}

void TestRhythmGrid::cleanupTestCase() {}

// Test adding cells to RhythmGrid
void TestRhythmGrid::testRhythmGridAddCell() {
  RhythmGrid grid;

  QVERIFY(grid.isEmpty());
  QCOMPARE(grid.cellCount(), 0);

  grid.addCell({"quarter", false, false});
  QCOMPARE(grid.cellCount(), 1);
  QVERIFY(!grid.isEmpty());

  grid.addCell({"eighth", false, false});
  QCOMPARE(grid.cellCount(), 2);

  grid.addCell({"quarter", false, true}); // Rest
  QCOMPARE(grid.cellCount(), 3);

  // Test convenience overload
  grid.addCell("half", true, false); // With tie
  QCOMPARE(grid.cellCount(), 4);
}

// Test removing cells from RhythmGrid
void TestRhythmGrid::testRhythmGridRemoveCell() {
  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", false, false});
  grid.addCell({"half", false, false});

  QCOMPARE(grid.cellCount(), 3);

  grid.removeLast();
  QCOMPARE(grid.cellCount(), 2);
  QCOMPARE(grid.cell(grid.cellCount() - 1).duration, QString("eighth"));

  grid.removeLast();
  QCOMPARE(grid.cellCount(), 1);
  QCOMPARE(grid.cell(0).duration, QString("quarter"));

  grid.removeLast();
  QVERIFY(grid.isEmpty());

  // Removing from empty grid should be safe
  grid.removeLast();
  QVERIFY(grid.isEmpty());
}

// Test clearing RhythmGrid
void TestRhythmGrid::testRhythmGridClear() {
  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", false, false});

  QCOMPARE(grid.cellCount(), 2);

  grid.clear();
  QVERIFY(grid.isEmpty());
  QCOMPARE(grid.cellCount(), 0);
}

// Test cell access by index
void TestRhythmGrid::testRhythmGridCellAccess() {
  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", true, false});  // With tie
  grid.addCell({"half", false, true});    // Rest

  QCOMPARE(grid.cell(0).duration, QString("quarter"));
  QCOMPARE(grid.cell(0).tie, false);
  QCOMPARE(grid.cell(0).isRest, false);

  QCOMPARE(grid.cell(1).duration, QString("eighth"));
  QCOMPARE(grid.cell(1).tie, true);
  QCOMPARE(grid.cell(1).isRest, false);

  QCOMPARE(grid.cell(2).duration, QString("half"));
  QCOMPARE(grid.cell(2).tie, false);
  QCOMPARE(grid.cell(2).isRest, true);

  // Out of bounds access should return default cell
  RhythmCell outOfBounds = grid.cell(99);
  QCOMPARE(outOfBounds.duration, QString("quarter")); // Default

  // Test setCell
  grid.setCell(1, {"dotted-quarter", false, false});
  QCOMPARE(grid.cell(1).duration, QString("dotted-quarter"));
  QCOMPARE(grid.cell(1).tie, false);

  // Test individual setters
  grid.setDuration(0, "sixteenth");
  QCOMPARE(grid.cell(0).duration, QString("sixteenth"));

  grid.setTie(0, true);
  QCOMPARE(grid.cell(0).tie, true);

  grid.setRest(0, true);
  QCOMPARE(grid.cell(0).isRest, true);
}

// Test RhythmGrid JSON serialization
void TestRhythmGrid::testRhythmGridSerialization() {
  RhythmGrid grid;
  grid.addCell({"quarter", false, false});
  grid.addCell({"eighth", true, false});
  grid.addCell({"half", false, true});

  // Serialize
  QJsonArray json = grid.toJson();
  QCOMPARE(json.size(), 3);

  // Deserialize
  RhythmGrid restored = RhythmGrid::fromJson(json);
  QCOMPARE(restored.cellCount(), 3);
  QCOMPARE(restored.cell(0).duration, QString("quarter"));
  QCOMPARE(restored.cell(1).tie, true);
  QCOMPARE(restored.cell(2).isRest, true);
}

// Test RhythmCell JSON serialization
void TestRhythmGrid::testRhythmCellSerialization() {
  RhythmCell cell{"dotted-quarter", true, false};

  QJsonObject json = cell.toJson();
  QCOMPARE(json["duration"].toString(), QString("dotted-quarter"));
  QCOMPARE(json["tie"].toBool(), true);
  QCOMPARE(json["isRest"].toBool(), false);

  RhythmCell restored = RhythmCell::fromJson(json);
  QCOMPARE(restored.duration, QString("dotted-quarter"));
  QCOMPARE(restored.tie, true);
  QCOMPARE(restored.isRest, false);
}

// Test total beats calculation
void TestRhythmGrid::testTotalBeats() {
  RhythmGrid grid;
  grid.addCell("quarter");   // 1 beat
  grid.addCell("eighth");    // 0.5 beat
  grid.addCell("half");      // 2 beats

  double total = grid.totalBeats();
  QCOMPARE(total, 3.5);

  grid.clear();
  grid.addCell("whole");
  QCOMPARE(grid.totalBeats(), 4.0);

  grid.clear();
  grid.addCell("sixteenth");
  QCOMPARE(grid.totalBeats(), 0.25);
}

// Test MotifEditorController::addNote
void TestRhythmGrid::testAddNote() {
  MotifEditorController controller;

  QSignalSpy pitchSpy(&controller, &MotifEditorController::pitchContourChanged);
  QSignalSpy rhythmSpy(&controller, &MotifEditorController::rhythmPatternChanged);

  QCOMPARE(controller.noteCount(), 0);

  controller.addNote(1); // Scale degree 1
  QCOMPARE(controller.noteCount(), 1);
  QCOMPARE(pitchSpy.count(), 1);
  QCOMPARE(rhythmSpy.count(), 1);

  controller.addNote(5); // Scale degree 5
  QCOMPARE(controller.noteCount(), 2);

  // Invalid scale degrees should be rejected
  controller.addNote(0);
  QCOMPARE(controller.noteCount(), 2); // No change

  controller.addNote(8);
  QCOMPARE(controller.noteCount(), 2); // No change

  // Check pitch contour
  QVariantList contour = controller.pitchContour();
  QCOMPARE(contour.size(), 2);
  QCOMPARE(contour[0].toInt(), 1);
  QCOMPARE(contour[1].toInt(), 5);
}

// Test MotifEditorController::addRhythm
void TestRhythmGrid::testAddRhythm() {
  MotifEditorController controller;

  controller.addNote(3);
  controller.addRhythm("eighth");

  QVariantList cells = controller.rhythmCells();
  QCOMPARE(cells.size(), 1);

  QVariantMap cell = cells[0].toMap();
  QCOMPARE(cell["duration"].toString(), QString("eighth"));
}

// Test MotifEditorController::removeLastNote
void TestRhythmGrid::testRemoveLastNote() {
  MotifEditorController controller;

  controller.addNote(1);
  controller.addNote(3);
  controller.addNote(5);

  QCOMPARE(controller.noteCount(), 3);

  controller.removeLastNote();
  QCOMPARE(controller.noteCount(), 2);

  controller.removeLastNote();
  QCOMPARE(controller.noteCount(), 1);

  controller.removeLastNote();
  QCOMPARE(controller.noteCount(), 0);

  // Removing from empty should be safe
  controller.removeLastNote();
  QCOMPARE(controller.noteCount(), 0);
}

// Test MotifEditorController::toggleRest
void TestRhythmGrid::testToggleRest() {
  MotifEditorController controller;

  controller.addNote(1);
  controller.addNote(3);

  // Initially not a rest
  QVERIFY(!controller.isRest(0));
  QVERIFY(!controller.isRest(1));

  // Toggle first note to rest
  controller.toggleRest(0);
  QVERIFY(controller.isRest(0));
  QVERIFY(!controller.isRest(1));

  // Toggle back
  controller.toggleRest(0);
  QVERIFY(!controller.isRest(0));

  // Out of bounds should be safe
  QVERIFY(!controller.isRest(99));
}

// Test MotifEditorController::toggleTie
void TestRhythmGrid::testToggleTie() {
  MotifEditorController controller;

  controller.addNote(1);
  controller.addNote(3);

  // Initially not tied
  QVERIFY(!controller.isTied(0));
  QVERIFY(!controller.isTied(1));

  // Toggle first note tie
  controller.toggleTie(0);
  QVERIFY(controller.isTied(0));

  // Toggle back
  controller.toggleTie(0);
  QVERIFY(!controller.isTied(0));

  // Out of bounds should be safe
  QVERIFY(!controller.isTied(99));
}

// Test MotifEditorController::clear
void TestRhythmGrid::testClear() {
  MotifEditorController controller;

  controller.addNote(1);
  controller.addNote(3);
  controller.addNote(5);

  QCOMPARE(controller.noteCount(), 3);

  controller.clear();
  QCOMPARE(controller.noteCount(), 0);
  QVERIFY(controller.pitchContour().isEmpty());
  QVERIFY(controller.rhythmCells().isEmpty());
}

// Test applying contour path generates rhythm cells
void TestRhythmGrid::testApplyContourPath() {
  MotifEditorController controller;
  controller.setMotifBars(1);

  // Create a simple contour path
  QVariantList path;
  for (int i = 0; i <= 10; i++) {
    QVariantMap point;
    point["x"] = i / 10.0;
    point["y"] = 0.5; // Middle pitch
    point["t"] = i * 0.1;
    path.append(point);
  }

  controller.applyContourPath(path);

  // Should have generated some notes
  QVERIFY(controller.noteCount() > 0);
  QVERIFY(controller.rhythmCells().size() > 0);

  // All should be notes (no rests for continuous line)
  QVariantList cells = controller.rhythmCells();
  int restCount = 0;
  for (const QVariant &v : cells) {
    QVariantMap cell = v.toMap();
    if (cell["isRest"].toBool())
      restCount++;
  }
  QCOMPARE(restCount, 0);
}

QTEST_MAIN(TestRhythmGrid)
#include "tst_rhythm_grid.moc"
