#pragma once

#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QMediaDevices>
#include <QObject>
#include <QTimer>
#include <memory>

// Voice types for different timbres
enum class VoiceType {
  SketchPiano = 0,  // Primary instrument for melody
  Soprano = 1,      // SATB soprano (flute-like)
  Alto = 2,         // SATB alto (string synth)
  Tenor = 3,        // SATB tenor (cello-like)
  Bass = 4,         // SATB bass (upright bass)
  Metronome = 5     // Click sound
};

// Audio engine with distinct voice patches
class AudioEngine : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
  Q_PROPERTY(int tempo READ tempo WRITE setTempo NOTIFY tempoChanged)

public:
  explicit AudioEngine(QObject *parent = nullptr);
  ~AudioEngine();

  // Play a note with a specific voice type
  Q_INVOKABLE void playNote(int midiPitch, int durationMs, int voiceType = 0);

  // Play multiple notes simultaneously (SATB chord)
  // Each note is: { midiPitch, voiceType }
  Q_INVOKABLE void playChord(const QVariantList &notes, int durationMs);
  
  // Pre-render and play an entire timeline of events
  // events: array of { startMs, durationMs, notes: [{midiPitch, voiceType}] }
  Q_INVOKABLE void playRenderedTimeline(const QVariantList &events, int totalDurationMs);

  // Play a click sound (for metronome)
  Q_INVOKABLE void playClick();

  // Stop all sound
  Q_INVOKABLE void stop();

  // Convert scale degree to MIDI pitch (with optional octave offset for SATB)
  Q_INVOKABLE static int scaleDegreeToMidi(int degree, const QString &key, int octaveOffset = 0);

  bool isPlaying() const { return m_isPlaying; }
  int tempo() const { return m_tempo; }
  void setTempo(int bpm);

signals:
  void isPlayingChanged();
  void tempoChanged();
  void noteFinished();

private slots:
  void onAudioOutputsChanged();

private:
  void initAudio();
  void recreateAudioSink();
  
  // Sound generation for each voice type
  QByteArray generateSketchPiano(int frequency, int durationMs);
  QByteArray generateSoprano(int frequency, int durationMs);
  QByteArray generateAlto(int frequency, int durationMs);
  QByteArray generateTenor(int frequency, int durationMs);
  QByteArray generateBass(int frequency, int durationMs);
  QByteArray generateMetronomeClick(int durationMs);
  
  // Waveform helpers
  float sineWave(float phase);
  float triangleWave(float phase);
  float sawtoothWave(float phase);
  float softSquareWave(float phase);
  
  int midiToFrequency(int midiPitch);

  std::unique_ptr<QAudioSink> m_audioSink;
  QBuffer m_audioBuffer;
  QAudioFormat m_format;
  QMediaDevices *m_mediaDevices = nullptr;
  QString m_currentDeviceId;
  bool m_isPlaying = false;
  int m_tempo = 120; // BPM

  // Key signatures: maps key name to root MIDI note
  static int keyToRootMidi(const QString &key);
};

