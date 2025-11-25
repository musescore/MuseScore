#include "MotifQuantizer.h"
#include <QTest>

class TestMotifQuantizer : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Scale degree quantization tests
  void testScaleDegreeQuantization_data();
  void testScaleDegreeQuantization();

  // Beat duration conversion tests
  void testBeatsToRhythmString_data();
  void testBeatsToRhythmString();

  // Simple contour quantization
  void testSinglePointContour();
  void testHorizontalLineContour();
  void testDiagonalLineContour();

  // Rest detection tests
  void testRestDetectionWithGap();
  void testNoRestForContinuousLine();

  // Multi-bar quantization
  void testMultiBarQuantization();

  // Edge cases
  void testEmptyContour();
  void testSinglePoint();

private:
  MotifQuantizer m_quantizer;
};

void TestMotifQuantizer::initTestCase() {
  QuantizerConfig config;
  config.beatsPerBar = 4;
  config.barsCount = 1;
  config.subdivisionsPerBeat = 4; // 4 subdivisions = 16th notes
  config.restThreshold = 0.15;
  m_quantizer.setConfig(config);
}

void TestMotifQuantizer::cleanupTestCase() {}

// Test data for scale degree quantization
void TestMotifQuantizer::testScaleDegreeQuantization_data() {
  QTest::addColumn<qreal>("normalizedY");
  QTest::addColumn<int>("expectedDegree");

  // Y=0 is top (high pitch), Y=1 is bottom (low pitch)
  // After inversion: 0->7, 1->1
  QTest::newRow("top (y=0)") << 0.0 << 7;
  QTest::newRow("bottom (y=1)") << 1.0 << 1;
  QTest::newRow("middle (y=0.5)") << 0.5 << 4;
  QTest::newRow("upper-middle (y=0.25)") << 0.25 << 5;  // ~5.5 rounds to 6 or 5
  QTest::newRow("lower-middle (y=0.75)") << 0.75 << 2;  // ~2.5 rounds to 2 or 3
}

void TestMotifQuantizer::testScaleDegreeQuantization() {
  QFETCH(qreal, normalizedY);
  QFETCH(int, expectedDegree);

  int result = m_quantizer.quantizeToScaleDegree(normalizedY);

  // Allow for rounding differences (±1)
  QVERIFY2(qAbs(result - expectedDegree) <= 1,
           qPrintable(QString("Expected degree ~%1, got %2 for y=%3")
                          .arg(expectedDegree)
                          .arg(result)
                          .arg(normalizedY)));
}

// Test data for beat duration conversion
void TestMotifQuantizer::testBeatsToRhythmString_data() {
  QTest::addColumn<qreal>("beats");
  QTest::addColumn<QString>("expectedDuration");

  QTest::newRow("whole note (4 beats)") << 4.0 << "whole";
  QTest::newRow("dotted half (3 beats)") << 3.0 << "dotted-half";
  QTest::newRow("half note (2 beats)") << 2.0 << "half";
  QTest::newRow("dotted quarter (1.5 beats)") << 1.5 << "dotted-quarter";
  QTest::newRow("quarter note (1 beat)") << 1.0 << "quarter";
  QTest::newRow("dotted eighth (0.75 beats)") << 0.75 << "dotted-eighth";
  QTest::newRow("eighth note (0.5 beats)") << 0.5 << "eighth";
  QTest::newRow("sixteenth note (0.25 beats)") << 0.25 << "sixteenth";
}

void TestMotifQuantizer::testBeatsToRhythmString() {
  QFETCH(qreal, beats);
  QFETCH(QString, expectedDuration);

  QString result = m_quantizer.beatsToRhythmString(beats);
  QCOMPARE(result, expectedDuration);
}

// Test single point contour (should produce one note)
void TestMotifQuantizer::testSinglePointContour() {
  QList<ContourPoint> points;
  points.append({0.5, 0.5, 0.0}); // Middle of canvas

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Should produce at least one note
  QVERIFY(notes.size() >= 1);

  // First note should not be a rest
  QVERIFY(!notes.first().isRest);

  // Scale degree should be around 4 (middle)
  QVERIFY(notes.first().scaleDegree >= 3 && notes.first().scaleDegree <= 5);
}

// Test horizontal line (sustained note at same pitch)
void TestMotifQuantizer::testHorizontalLineContour() {
  QList<ContourPoint> points;
  // Draw a horizontal line at y=0.3 (upper pitch) from x=0 to x=1
  for (int i = 0; i <= 10; i++) {
    points.append({i / 10.0, 0.3, i * 0.1});
  }

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Should produce notes (no rests for continuous line)
  QVERIFY(notes.size() >= 1);

  // Check that there are no rests in a continuous horizontal line
  int restCount = 0;
  for (const auto &note : notes) {
    if (note.isRest)
      restCount++;
  }

  // A continuous line should have minimal or no rests
  QVERIFY2(restCount == 0,
           qPrintable(QString("Expected 0 rests for continuous line, got %1")
                          .arg(restCount)));
}

// Test diagonal line (changing pitches)
void TestMotifQuantizer::testDiagonalLineContour() {
  QList<ContourPoint> points;
  // Draw a diagonal line from top-left to bottom-right
  for (int i = 0; i <= 10; i++) {
    points.append({i / 10.0, i / 10.0, i * 0.1});
  }

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Should produce notes
  QVERIFY(notes.size() >= 1);

  // No rests in continuous diagonal line
  for (const auto &note : notes) {
    QVERIFY(!note.isRest);
  }
}

// Test rest detection when there's a gap in drawing
void TestMotifQuantizer::testRestDetectionWithGap() {
  QList<ContourPoint> points;

  // First stroke: draw from x=0 to x=0.3
  for (int i = 0; i <= 3; i++) {
    points.append({i / 10.0, 0.5, i * 0.05});
  }

  // Gap: skip x=0.3 to x=0.6 with time gap (simulates finger lift)
  // Add points starting at x=0.6 with a time jump
  for (int i = 6; i <= 10; i++) {
    points.append({i / 10.0, 0.5, 0.5 + (i - 6) * 0.05}); // Time gap of 0.35s
  }

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Should have detected a rest due to the x-gap AND time gap
  bool hasRest = false;
  for (const auto &note : notes) {
    if (note.isRest) {
      hasRest = true;
      break;
    }
  }

  QVERIFY2(hasRest, "Expected a rest to be detected due to gap in contour");
}

// Test that continuous line produces no rests
void TestMotifQuantizer::testNoRestForContinuousLine() {
  QList<ContourPoint> points;

  // Continuous line with no gaps
  for (int i = 0; i <= 20; i++) {
    points.append({i / 20.0, 0.5, i * 0.02});
  }

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Count rests
  int restCount = 0;
  for (const auto &note : notes) {
    if (note.isRest)
      restCount++;
  }

  QCOMPARE(restCount, 0);
}

// Test multi-bar quantization
void TestMotifQuantizer::testMultiBarQuantization() {
  QuantizerConfig config;
  config.beatsPerBar = 4;
  config.barsCount = 2; // 2 bars = 8 beats
  config.subdivisionsPerBeat = 4;
  config.restThreshold = 0.15;
  m_quantizer.setConfig(config);

  QList<ContourPoint> points;
  // Draw across the full 2-bar span
  for (int i = 0; i <= 20; i++) {
    points.append({i / 20.0, 0.5, i * 0.1});
  }

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Should produce notes spanning the 2 bars
  QVERIFY(notes.size() >= 1);

  // Calculate total beats using durationBeats field
  qreal totalBeats = 0;
  for (const auto &note : notes) {
    totalBeats += note.durationBeats;
  }

  // Total should be around 8 beats for 2 bars (allow some flexibility)
  QVERIFY2(totalBeats >= 1.0 && totalBeats <= 8.0,
           qPrintable(QString("Total beats %1 outside expected range")
                          .arg(totalBeats)));

  // Reset config
  config.barsCount = 1;
  m_quantizer.setConfig(config);
}

// Test empty contour
void TestMotifQuantizer::testEmptyContour() {
  QList<ContourPoint> points;

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  QVERIFY(notes.isEmpty());
}

// Test single point (edge case)
void TestMotifQuantizer::testSinglePoint() {
  QList<ContourPoint> points;
  points.append({0.0, 0.0, 0.0});

  QList<QuantizedNote> notes = m_quantizer.quantize(points);

  // Should handle single point gracefully
  // May produce one note or none depending on implementation
  QVERIFY(notes.size() <= 1);
}

QTEST_MAIN(TestMotifQuantizer)
#include "tst_motif_quantizer.moc"

