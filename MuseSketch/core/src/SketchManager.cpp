#include "SketchManager.h"
#include "Motif.h"
#include "PartwritingEngine.h"
#include "RhythmGrid.h"
#include "Section.h"
#include "Sketch.h"
#include <QUuid>
#include <QVariantMap>

SketchManager::SketchManager(QObject *parent) : QObject(parent) {
  m_partwritingEngine = new PartwritingEngine();
  loadSketches();
}

SketchManager::~SketchManager() {
  delete m_partwritingEngine;
}

QString SketchManager::createNewSketch(const QString &name) {
  QString id = m_repository.createSketch(name);
  loadSketches();
  return id;
}

void SketchManager::openSketch(const QString &id) { emit sketchOpened(id); }

void SketchManager::deleteSketch(const QString &id) {
  m_repository.deleteSketch(id);
  loadSketches();
}

void SketchManager::refreshSketches() { loadSketches(); }

QVariantMap SketchManager::getSketch(const QString &id) {
  // Force reload from disk to get the latest data (other components may have modified it)
  Sketch sketch = m_repository.loadSketch(id, true);

  QVariantMap sketchMap;
  sketchMap["id"] = sketch.id();
  sketchMap["name"] = sketch.name();
  sketchMap["key"] = sketch.key();
  sketchMap["tempo"] = sketch.tempo();
  sketchMap["timeSignature"] = sketch.timeSignature();

  // Convert motifs to QVariantList
  QVariantList motifsArray;
  for (const Motif &motif : sketch.motifs()) {
    QVariantMap motifMap;
    motifMap["id"] = motif.id();
    motifMap["name"] = motif.name();
    motifMap["lengthBars"] = motif.lengthBars();

    // Convert pitch contour
    QVariantList pitchArray;
    for (int degree : motif.pitchContour()) {
      pitchArray.append(degree);
    }
    motifMap["pitchContour"] = pitchArray;

    // Convert rhythm pattern (legacy format for compatibility)
    QVariantList rhythmArray;
    for (const QString &rhythm : motif.rhythmPattern()) {
      rhythmArray.append(rhythm);
    }
    motifMap["rhythmPattern"] = rhythmArray;

    // Convert rhythm cells (new format with tie/rest info)
    QVariantList rhythmCells;
    for (const RhythmCell &cell : motif.rhythmGrid().cells()) {
      QVariantMap cellMap;
      cellMap["duration"] = cell.duration;
      cellMap["tie"] = cell.tie;
      cellMap["isRest"] = cell.isRest;
      rhythmCells.append(cellMap);
    }
    motifMap["rhythmCells"] = rhythmCells;

    motifsArray.append(motifMap);
  }
  sketchMap["motifs"] = motifsArray;

  return sketchMap;
}

QVariantMap SketchManager::getMotif(const QString &sketchId, const QString &motifId) {
  Sketch sketch = m_repository.loadSketch(sketchId, true);
  
  for (const Motif &motif : sketch.motifs()) {
    if (motif.id() == motifId) {
      QVariantMap motifMap;
      motifMap["id"] = motif.id();
      motifMap["name"] = motif.name();
      motifMap["lengthBars"] = motif.lengthBars();
      
      // Convert pitch contour
      QVariantList pitchArray;
      for (int degree : motif.pitchContour()) {
        pitchArray.append(degree);
      }
      motifMap["pitchContour"] = pitchArray;
      
      // Convert rhythm pattern
      QVariantList rhythmArray;
      for (const QString &rhythm : motif.rhythmPattern()) {
        rhythmArray.append(rhythm);
      }
      motifMap["rhythmPattern"] = rhythmArray;
      
      // Convert rhythm cells
      QVariantList rhythmCells;
      for (const RhythmCell &cell : motif.rhythmGrid().cells()) {
        QVariantMap cellMap;
        cellMap["duration"] = cell.duration;
        cellMap["tie"] = cell.tie;
        cellMap["isRest"] = cell.isRest;
        rhythmCells.append(cellMap);
      }
      motifMap["rhythmCells"] = rhythmCells;
      
      return motifMap;
    }
  }
  
  return QVariantMap(); // Not found
}

void SketchManager::updateMotifPitches(const QString &sketchId, const QString &motifId, const QVariantList &pitches) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  QList<int> newPitches;
  for (const QVariant &p : pitches) {
    newPitches.append(p.toInt());
  }
  
  // Find and update the motif
  QList<Motif> motifs = sketch.motifs();
  for (int i = 0; i < motifs.size(); ++i) {
    if (motifs[i].id() == motifId) {
      Motif updatedMotif = motifs[i];
      updatedMotif.setPitchContour(newPitches);
      motifs[i] = updatedMotif;
      break;
    }
  }
  
  // Update sketch with modified motifs
  sketch.setMotifs(motifs);
  m_repository.saveSketch(sketch);
  
  loadSketches();
}

void SketchManager::renameMotif(const QString &sketchId, const QString &motifId, const QString &newName) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  QList<Motif> motifs = sketch.motifs();
  for (int i = 0; i < motifs.size(); ++i) {
    if (motifs[i].id() == motifId) {
      motifs[i].setName(newName);
      break;
    }
  }
  
  sketch.setMotifs(motifs);
  m_repository.saveSketch(sketch);
  loadSketches();
}

QString SketchManager::duplicateMotif(const QString &sketchId, const QString &motifId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  // Find the original motif
  const Motif *original = nullptr;
  for (const Motif &m : sketch.motifs()) {
    if (m.id() == motifId) {
      original = &m;
      break;
    }
  }
  
  if (!original) {
    return QString();
  }
  
  // Create a duplicate with a new ID
  QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
  Motif duplicate(newId, original->name() + " (copy)", original->lengthBars());
  duplicate.setPitchContour(original->pitchContour());
  duplicate.setRhythmGrid(original->rhythmGrid());
  duplicate.setKeyRef(original->keyRef());
  
  sketch.addMotif(duplicate);
  m_repository.saveSketch(sketch);
  loadSketches();
  
  return newId;
}

void SketchManager::deleteMotif(const QString &sketchId, const QString &motifId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  sketch.removeMotif(motifId);
  m_repository.saveSketch(sketch);
  loadSketches();
}

// Section CRUD operations (PR-07)

QString SketchManager::createSection(const QString &sketchId, const QString &name, int lengthBars) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  QString sectionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
  Section section(sectionId, name, lengthBars);
  sketch.addSection(section);
  
  m_repository.saveSketch(sketch);
  return sectionId;
}

QVariantMap SketchManager::getSection(const QString &sketchId, const QString &sectionId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  for (const Section &section : sketch.sections()) {
    if (section.id() == sectionId) {
      QVariantMap sectionMap;
      sectionMap["id"] = section.id();
      sectionMap["name"] = section.name();
      sectionMap["lengthBars"] = section.lengthBars();
      
      // Convert placements
      QVariantList placementsArray;
      for (const MotifPlacement &p : section.placements()) {
        QVariantMap pMap;
        pMap["motifId"] = p.motifId;
        pMap["startBar"] = p.startBar;
        pMap["repetitions"] = p.repetitions;
        pMap["voice"] = p.voice;
        
        // Include motif info for display
        for (const Motif &m : sketch.motifs()) {
          if (m.id() == p.motifId) {
            pMap["motifName"] = m.name();
            pMap["motifLengthBars"] = m.lengthBars();
            pMap["endBar"] = p.endBar(m.lengthBars());
            break;
          }
        }
        
        placementsArray.append(pMap);
      }
      sectionMap["placements"] = placementsArray;
      
      return sectionMap;
    }
  }
  
  return QVariantMap();
}

QVariantList SketchManager::getSections(const QString &sketchId) {
  Sketch sketch = m_repository.loadSketch(sketchId, true);
  
  QVariantList sectionsArray;
  for (const Section &section : sketch.sections()) {
    QVariantMap sectionMap;
    sectionMap["id"] = section.id();
    sectionMap["name"] = section.name();
    sectionMap["lengthBars"] = section.lengthBars();
    sectionMap["placementCount"] = section.placements().size();
    sectionsArray.append(sectionMap);
  }
  
  return sectionsArray;
}

void SketchManager::renameSection(const QString &sketchId, const QString &sectionId, const QString &newName) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    section->setName(newName);
    m_repository.saveSketch(sketch);
  }
}

void SketchManager::setSectionLength(const QString &sketchId, const QString &sectionId, int lengthBars) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    section->setLengthBars(lengthBars);
    m_repository.saveSketch(sketch);
  }
}

void SketchManager::deleteSection(const QString &sketchId, const QString &sectionId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  sketch.removeSection(sectionId);
  m_repository.saveSketch(sketch);
}

void SketchManager::addPlacement(const QString &sketchId, const QString &sectionId,
                                  const QString &motifId, int startBar, int repetitions) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    MotifPlacement placement;
    placement.motifId = motifId;
    placement.startBar = startBar;
    placement.repetitions = repetitions;
    placement.voice = 0; // Default to melody voice
    
    section->addPlacement(placement);
    m_repository.saveSketch(sketch);
  }
}

void SketchManager::removePlacement(const QString &sketchId, const QString &sectionId, int placementIndex) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    section->removePlacement(placementIndex);
    m_repository.saveSketch(sketch);
  }
}

void SketchManager::movePlacement(const QString &sketchId, const QString &sectionId,
                                   int placementIndex, int newStartBar) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    section->movePlacement(placementIndex, newStartBar);
    m_repository.saveSketch(sketch);
  }
}

void SketchManager::setPlacementRepetitions(const QString &sketchId, const QString &sectionId,
                                             int placementIndex, int repetitions) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    section->setPlacementRepetitions(placementIndex, repetitions);
    m_repository.saveSketch(sketch);
  }
}

QVariantList SketchManager::getSectionTimeline(const QString &sketchId, const QString &sectionId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  for (const Section &section : sketch.sections()) {
    if (section.id() == sectionId) {
      QList<NoteEvent> timeline = section.flattenToTimeline(sketch);
      
      QVariantList result;
      for (const NoteEvent &event : timeline) {
        QVariantMap eventMap;
        eventMap["scaleDegree"] = event.scaleDegree;
        eventMap["duration"] = event.duration;
        eventMap["startBeat"] = event.startBeat;
        eventMap["isRest"] = event.isRest;
        eventMap["tie"] = event.tie;
        result.append(eventMap);
      }
      return result;
    }
  }
  
  return QVariantList();
}

// Texture/SATB operations (PR-08)

void SketchManager::setSectionTexture(const QString &sketchId, const QString &sectionId,
                                       const QString &textureType) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  Section *section = sketch.findSection(sectionId);
  if (section) {
    if (textureType == "SATBChorale") {
      section->setTextureType(TextureType::SATBChorale);
    } else {
      section->setTextureType(TextureType::MelodyOnly);
    }
    m_repository.saveSketch(sketch);
  }
}

QString SketchManager::getSectionTexture(const QString &sketchId, const QString &sectionId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  for (const Section &section : sketch.sections()) {
    if (section.id() == sectionId) {
      return section.textureTypeString();
    }
  }
  return "MelodyOnly";
}

void SketchManager::generateSATBChorale(const QString &sketchId, const QString &sectionId,
                                         const QString &sopranoMotifId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  // Find the soprano motif
  const Motif *sopranoMotif = nullptr;
  for (const Motif &m : sketch.motifs()) {
    if (m.id() == sopranoMotifId) {
      sopranoMotif = &m;
      break;
    }
  }
  
  if (!sopranoMotif) return;
  
  Section *section = sketch.findSection(sectionId);
  if (!section) return;
  
  // Generate SATB voices using the partwriting engine
  ChoraleVoices voices = m_partwritingEngine->generateChoraleTexture(*sopranoMotif, sketch);
  
  // Store the voices in the section
  VoiceData soprano, alto, tenor, bass;
  
  soprano.pitches = voices.soprano.pitches;
  soprano.rhythm = voices.soprano.rhythm;
  
  alto.pitches = voices.alto.pitches;
  alto.rhythm = voices.alto.rhythm;
  
  tenor.pitches = voices.tenor.pitches;
  tenor.rhythm = voices.tenor.rhythm;
  
  bass.pitches = voices.bass.pitches;
  bass.rhythm = voices.bass.rhythm;
  
  section->setSopranoVoice(soprano);
  section->setAltoVoice(alto);
  section->setTenorVoice(tenor);
  section->setBassVoice(bass);
  section->setTextureType(TextureType::SATBChorale);
  
  m_repository.saveSketch(sketch);
}

void SketchManager::generateSATBFromTimeline(const QString &sketchId, const QString &sectionId) {
  Sketch sketch = m_repository.loadSketch(sketchId, true);
  
  Section *section = sketch.findSection(sectionId);
  if (!section) return;
  
  // Get the melody timeline from the section's placements
  QList<NoteEvent> timeline = section->flattenToTimeline(sketch);
  
  if (timeline.isEmpty()) {
    qWarning() << "Cannot generate SATB from empty timeline";
    return;
  }
  
  // Extract pitches and rhythm from the timeline
  QList<int> pitches;
  RhythmGrid rhythm;
  
  for (const NoteEvent &event : timeline) {
    if (!event.isRest && event.scaleDegree > 0) {
      pitches.append(event.scaleDegree);
    }
    
    RhythmCell cell;
    cell.duration = event.duration;
    cell.isRest = event.isRest;
    cell.tie = event.tie;
    rhythm.addCell(cell);
  }
  
  if (pitches.isEmpty()) {
    qWarning() << "No notes in timeline to generate SATB from";
    return;
  }
  
  // Generate SATB voices using the partwriting engine
  ChoraleVoices voices = m_partwritingEngine->generateChoraleFromPitches(pitches, rhythm, sketch.key());
  
  // Store the voices in the section
  VoiceData soprano, alto, tenor, bass;
  
  soprano.pitches = voices.soprano.pitches;
  soprano.rhythm = voices.soprano.rhythm;
  
  alto.pitches = voices.alto.pitches;
  alto.rhythm = voices.alto.rhythm;
  
  tenor.pitches = voices.tenor.pitches;
  tenor.rhythm = voices.tenor.rhythm;
  
  bass.pitches = voices.bass.pitches;
  bass.rhythm = voices.bass.rhythm;
  
  section->setSopranoVoice(soprano);
  section->setAltoVoice(alto);
  section->setTenorVoice(tenor);
  section->setBassVoice(bass);
  section->setTextureType(TextureType::SATBChorale);
  
  m_repository.saveSketch(sketch);
  
  qDebug() << "Generated SATB from timeline with" << pitches.size() << "notes";
}

QVariantList SketchManager::getSATBVoiceTimeline(const QString &sketchId, const QString &sectionId,
                                                  int voiceIndex) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  for (const Section &section : sketch.sections()) {
    if (section.id() == sectionId) {
      QList<NoteEvent> timeline = section.flattenVoiceToTimeline(voiceIndex);
      
      QVariantList result;
      for (const NoteEvent &event : timeline) {
        QVariantMap eventMap;
        eventMap["scaleDegree"] = event.scaleDegree;
        eventMap["duration"] = event.duration;
        eventMap["startBeat"] = event.startBeat;
        eventMap["isRest"] = event.isRest;
        eventMap["tie"] = event.tie;
        result.append(eventMap);
      }
      return result;
    }
  }
  
  return QVariantList();
}

bool SketchManager::hasSATBVoices(const QString &sketchId, const QString &sectionId) {
  Sketch sketch = m_repository.loadSketch(sketchId);
  
  for (const Section &section : sketch.sections()) {
    if (section.id() == sectionId) {
      return section.hasSATBVoices();
    }
  }
  return false;
}

QVariantList SketchManager::sketches() const { return m_sketches; }

void SketchManager::loadSketches() {
  QList<SketchInfo> sketchInfos = m_repository.listSketches();

  m_sketches.clear();
  for (const SketchInfo &info : sketchInfos) {
    QVariantMap sketchMap;
    sketchMap["id"] = info.id;
    sketchMap["name"] = info.name;
    sketchMap["modifiedAt"] = info.modifiedAt.toString("yyyy-MM-dd hh:mm");
    m_sketches.append(sketchMap);
  }

  emit sketchesChanged();
}
