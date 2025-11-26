#include "MotifPlayer.h"
#include <QDebug>

MotifPlayer::MotifPlayer(QObject *parent) : QObject(parent) {
  m_playbackTimer.setSingleShot(true);
  connect(&m_playbackTimer, &QTimer::timeout, this, &MotifPlayer::playNextNote);

  connect(&m_metronomeTimer, &QTimer::timeout, this,
          &MotifPlayer::onMetronomeTick);
  
  // Timer for prerendered playback end
  m_prerenderedEndTimer.setSingleShot(true);
  connect(&m_prerenderedEndTimer, &QTimer::timeout, this, [this]() {
    qDebug() << "MotifPlayer: Prerendered playback ended, isPlaying=" << m_isPlaying << "isLooping=" << m_isLooping;
    if (m_isPlaying) {
      if (m_isLooping) {
        // Restart for looping
        playPrerendered();
      } else {
        // Properly stop playback
        m_audioEngine.stop();
        m_isPlaying = false;
        m_currentNoteIndex = 0;
        m_currentPitchIndex = 0;
        m_currentBeat = 0;
        emit isPlayingChanged();
        emit currentNoteIndexChanged();
        emit playbackFinished();
        qDebug() << "MotifPlayer: Emitted isPlayingChanged and playbackFinished";
      }
    }
  });
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

void MotifPlayer::playPrerendered() {
  if (m_rhythmCells.isEmpty()) {
    qWarning() << "MotifPlayer: No motif to play (empty rhythm cells)";
    return;
  }
  
  if (m_pitchContour.isEmpty()) {
    qWarning() << "MotifPlayer: No motif to play (empty pitch contour)";
    return;
  }

  qDebug() << "MotifPlayer::playPrerendered() - rhythmCells:" << m_rhythmCells.size() 
           << "pitches:" << m_pitchContour.size() << "tempo:" << m_tempo;

  // Build event list for pre-rendering
  QVariantList events;
  int msPerBeat = 60000 / m_tempo;
  int currentTimeMs = 0;
  int pitchIndex = 0;
  int totalDurationMs = 0;
  
  for (int i = 0; i < m_rhythmCells.size(); i++) {
    const RhythmCell &cell = m_rhythmCells[i];
    int durationMs = durationToMs(cell.duration);
    
    qDebug() << "  Cell" << i << ": duration=" << cell.duration 
             << "isRest=" << cell.isRest << "durationMs=" << durationMs;
    
    if (!cell.isRest && pitchIndex < m_pitchContour.size()) {
      int scaleDegree = m_pitchContour[pitchIndex];
      int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, m_key);
      
      qDebug() << "    -> Note: scaleDegree=" << scaleDegree 
               << "midiPitch=" << midiPitch << "startMs=" << currentTimeMs;
      
      QVariantList notes;
      QVariantMap note;
      note["midiPitch"] = midiPitch;
      note["voiceType"] = m_voiceType;
      notes.append(note);
      
      QVariantMap event;
      event["startMs"] = currentTimeMs;
      event["durationMs"] = qMax(50, durationMs - 30); // Slight gap, but at least 50ms
      event["notes"] = notes;
      events.append(event);
      
      pitchIndex++;
    }
    
    // Advance time for ALL cells (notes and rests)
    currentTimeMs += durationMs;
  }
  
  totalDurationMs = currentTimeMs;
  
  // Add metronome clicks to the event list if enabled
  if (m_metronomeEnabled) {
    int numBeats = (totalDurationMs + msPerBeat - 1) / msPerBeat;
    qDebug() << "Adding" << numBeats << "metronome clicks";
    for (int beat = 0; beat < numBeats; beat++) {
      QVariantList notes;
      QVariantMap note;
      note["midiPitch"] = -1;  // Special value for metronome click
      note["voiceType"] = 5;   // VoiceType::Metronome
      notes.append(note);
      
      QVariantMap event;
      event["startMs"] = beat * msPerBeat;
      event["durationMs"] = 50;  // Short click
      event["notes"] = notes;
      events.append(event);
    }
  }
  
  qDebug() << "MotifPlayer: Built" << events.size() << "events, totalDuration=" << totalDurationMs << "ms";
  
  if (events.isEmpty()) {
    qWarning() << "MotifPlayer: No notes to play";
    return;
  }
  
  m_isPlaying = true;
  m_currentNoteIndex = 0;
  m_currentPitchIndex = 0;
  m_currentBeat = 0;
  emit isPlayingChanged();
  
  // Play the pre-rendered timeline (includes metronome if enabled)
  m_audioEngine.playRenderedTimeline(events, totalDurationMs);
  
  // Schedule playback finished using the member timer (can be cancelled by stop())
  m_prerenderedEndTimer.start(totalDurationMs + 100);
}

void MotifPlayer::stop() {
  m_playbackTimer.stop();
  m_prerenderedEndTimer.stop();  // Cancel any pending prerendered end callback
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
  m_audioEngine.playNote(midiPitch, 300, m_voiceType); // 300ms preview with current voice
}

void MotifPlayer::playPreviewNoteWithVoice(int scaleDegree, const QString &key, int voiceType) {
  int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, key);
  m_audioEngine.playNote(midiPitch, 300, voiceType);
}

void MotifPlayer::playSATBChord(const QVariantList &notes, const QString &key, int durationMs) {
  // Convert scale degrees to MIDI pitches and create chord data
  QVariantList chordNotes;
  
  for (int idx = 0; idx < notes.size(); idx++) {
    QVariantMap noteMap = notes[idx].toMap();
    int scaleDegree = noteMap["scaleDegree"].toInt();
    int voiceType = noteMap["voiceType"].toInt();
    int octaveOffset = noteMap["octaveOffset"].toInt();
    
    if (scaleDegree > 0) {
      int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, key, octaveOffset);
      
      QVariantMap chordNote;
      chordNote["midiPitch"] = midiPitch;
      chordNote["voiceType"] = voiceType;
      chordNotes.append(chordNote);
    }
  }
  
  if (!chordNotes.isEmpty()) {
    m_audioEngine.playChord(chordNotes, durationMs);
  }
}

void MotifPlayer::playSATBTimeline(const QVariantList &events, const QString &key) {
  // Convert events from beat-based to millisecond-based and scale degrees to MIDI
  QVariantList renderedEvents;
  int msPerBeat = 60000 / m_tempo;
  int totalDurationMs = 0;
  
  for (int eventIdx = 0; eventIdx < events.size(); eventIdx++) {
    QVariantMap event = events[eventIdx].toMap();
    double startBeat = event["startBeat"].toDouble();
    QString duration = event["duration"].toString();
    QVariantList notes = event["notes"].toList();
    
    // Convert beat to milliseconds
    int startMs = int(startBeat * msPerBeat);
    int durationMs = durationToMs(duration);
    
    // Track total duration
    if (startMs + durationMs > totalDurationMs) {
      totalDurationMs = startMs + durationMs;
    }
    
    // Convert notes from scale degrees to MIDI
    QVariantList midiNotes;
    for (int noteIdx = 0; noteIdx < notes.size(); noteIdx++) {
      QVariantMap noteMap = notes[noteIdx].toMap();
      int scaleDegree = noteMap["scaleDegree"].toInt();
      int voiceType = noteMap["voiceType"].toInt();
      int octaveOffset = noteMap["octaveOffset"].toInt();
      
      if (scaleDegree > 0) {
        int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, key, octaveOffset);
        
        QVariantMap midiNote;
        midiNote["midiPitch"] = midiPitch;
        midiNote["voiceType"] = voiceType;
        midiNotes.append(midiNote);
      }
    }
    
    if (!midiNotes.isEmpty()) {
      QVariantMap renderedEvent;
      renderedEvent["startMs"] = startMs;
      renderedEvent["durationMs"] = durationMs;
      renderedEvent["notes"] = midiNotes;
      renderedEvents.append(renderedEvent);
    }
  }
  
  if (!renderedEvents.isEmpty()) {
    m_isPlaying = true;
    m_currentNoteIndex = 0;
    m_currentPitchIndex = 0;
    m_currentBeat = 0;
    emit isPlayingChanged();
    
    m_audioEngine.playRenderedTimeline(renderedEvents, totalDurationMs);
    
    // Schedule playback finished using the member timer (can be cancelled by stop())
    m_prerenderedEndTimer.start(totalDurationMs + 100);
  }
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
    // Play the note with current voice type
    int scaleDegree = m_pitchContour[m_currentPitchIndex];
    int midiPitch = AudioEngine::scaleDegreeToMidi(scaleDegree, m_key);

    // Slightly shorter than full duration for articulation
    int noteDurationMs = qMax(50, durationMs - 50);
    m_audioEngine.playNote(midiPitch, noteDurationMs, m_voiceType);

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

void MotifPlayer::setVoiceType(int type) {
  if (type >= 0 && type <= 5 && type != m_voiceType) {
    m_voiceType = type;
    emit voiceTypeChanged();
  }
}

