#include "MotifPlayer.h"
#include <QDebug>

MotifPlayer::MotifPlayer(QObject *parent) : QObject(parent) {
  m_playbackTimer.setSingleShot(true);
  connect(&m_playbackTimer, &QTimer::timeout, this, &MotifPlayer::playNextNote);

  connect(&m_metronomeTimer, &QTimer::timeout, this,
          &MotifPlayer::onMetronomeTick);
}

void MotifPlayer::setMotif(const QVariantList &pitchContour,
                           const QVariantList &rhythmCells,
                           const QString &key) {
  stop();

  m_pitchContour.clear();
  m_rhythmCells.clear();

  for (const QVariant &v : pitchContour) {
    m_pitchContour.append(v.toInt());
  }

  for (const QVariant &v : rhythmCells) {
    QVariantMap map = v.toMap();
    RhythmCell cell;
    cell.duration = map["duration"].toString();
    cell.tie = map["tie"].toBool();
    cell.isRest = map["isRest"].toBool();
    m_rhythmCells.append(cell);
  }

  m_key = key;

  qDebug() << "MotifPlayer: Set motif with" << m_pitchContour.size()
           << "pitches and" << m_rhythmCells.size() << "rhythm cells";
}

void MotifPlayer::play() {
  if (m_rhythmCells.isEmpty()) {
    qWarning() << "MotifPlayer: No motif to play";
    return;
  }

  if (m_isPaused) {
    // Resume from current position
    m_isPaused = false;
  } else {
    // Start from beginning
    m_currentNoteIndex = 0;
    m_currentPitchIndex = 0;
    m_currentBeat = 0;
  }

  m_isPlaying = true;
  emit isPlayingChanged();

  if (m_metronomeEnabled) {
    startMetronome();
  }

  // Play first note immediately
  playNextNote();
}

void MotifPlayer::stop() {
  m_playbackTimer.stop();
  stopMetronome();
  m_audioEngine.stop();

  m_isPlaying = false;
  m_isPaused = false;
  m_currentNoteIndex = 0;
  m_currentPitchIndex = 0;
  m_currentBeat = 0;

  emit isPlayingChanged();
  emit currentNoteIndexChanged();
}

void MotifPlayer::pause() {
  if (m_isPlaying) {
    m_playbackTimer.stop();
    stopMetronome();
    m_audioEngine.stop();
    m_isPlaying = false;
    m_isPaused = true;
    emit isPlayingChanged();
  }
}

void MotifPlayer::playPreviewNote(int scaleDegree, const QString &key) {
  int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, key);
  m_audioEngine.playNote(midiPitch, 300); // 300ms preview
}

void MotifPlayer::playNextNote() {
  if (!m_isPlaying || m_currentNoteIndex >= m_rhythmCells.size()) {
    if (m_isLooping && !m_rhythmCells.isEmpty()) {
      // Loop back to start
      m_currentNoteIndex = 0;
      m_currentPitchIndex = 0;
      m_currentBeat = 0;
      playNextNote();
      return;
    }

    // Playback finished
    stop();
    emit playbackFinished();
    return;
  }

  const RhythmCell &cell = m_rhythmCells[m_currentNoteIndex];
  int durationMs = durationToMs(cell.duration);

  if (!cell.isRest && m_currentPitchIndex < m_pitchContour.size()) {
    // Play the note
    int scaleDegree = m_pitchContour[m_currentPitchIndex];
    int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, m_key);

    // Slightly shorter than full duration for articulation
    int noteDurationMs = qMax(50, durationMs - 50);
    m_audioEngine.playNote(midiPitch, noteDurationMs);

    m_currentPitchIndex++;
  }
  // If it's a rest, we just wait

  emit currentNoteIndexChanged();

  // Schedule next note
  m_currentNoteIndex++;
  m_playbackTimer.start(durationMs);
}

void MotifPlayer::onMetronomeTick() {
  m_currentBeat++;
  m_audioEngine.playClick();
  emit beatTick(m_currentBeat);
}

void MotifPlayer::startMetronome() {
  int msPerBeat = 60000 / m_tempo;
  m_currentBeat = 0;

  // Play first click immediately
  m_audioEngine.playClick();
  emit beatTick(0);

  m_metronomeTimer.start(msPerBeat);
}

void MotifPlayer::stopMetronome() { m_metronomeTimer.stop(); }

int MotifPlayer::durationToMs(const QString &duration) {
  // Convert duration string to milliseconds based on tempo
  // Quarter note = one beat = 60000/tempo ms

  int msPerBeat = 60000 / m_tempo;

  if (duration == "whole")
    return msPerBeat * 4;
  if (duration == "dotted-half")
    return msPerBeat * 3;
  if (duration == "half")
    return msPerBeat * 2;
  if (duration == "dotted-quarter")
    return msPerBeat * 3 / 2;
  if (duration == "quarter")
    return msPerBeat;
  if (duration == "dotted-eighth")
    return msPerBeat * 3 / 4;
  if (duration == "eighth")
    return msPerBeat / 2;
  if (duration == "sixteenth")
    return msPerBeat / 4;

  return msPerBeat; // Default to quarter
}

void MotifPlayer::setLooping(bool loop) {
  if (m_isLooping != loop) {
    m_isLooping = loop;
    emit loopingChanged();
  }
}

void MotifPlayer::setTempo(int bpm) {
  if (bpm >= 40 && bpm <= 240 && bpm != m_tempo) {
    m_tempo = bpm;
    m_audioEngine.setTempo(bpm);

    // Update metronome interval if running
    if (m_metronomeTimer.isActive()) {
      int msPerBeat = 60000 / m_tempo;
      m_metronomeTimer.setInterval(msPerBeat);
    }

    emit tempoChanged();
  }
}

void MotifPlayer::setMetronomeEnabled(bool enabled) {
  if (m_metronomeEnabled != enabled) {
    m_metronomeEnabled = enabled;

    if (m_isPlaying) {
      if (enabled) {
        startMetronome();
      } else {
        stopMetronome();
      }
    }

    emit metronomeEnabledChanged();
  }
}

