#pragma once

#include "Sketch.h"
#include "SketchRepository.h"
#include <QObject>
#include <QString>
#include <QUrl>

/**
 * ExportEngine handles exporting sketches to various formats:
 * - MIDI (.mid) - Standard MIDI file format
 * - MusicXML (.musicxml) - MusicXML 3.1 format
 * - MuseScore (.mscz) - MuseScore project format (future integration)
 */
class ExportEngine : public QObject {
    Q_OBJECT

public:
    explicit ExportEngine(QObject *parent = nullptr);
    ~ExportEngine() = default;

    // Export formats
    enum class Format {
        MIDI,
        MusicXML,
        MSCZ
    };
    Q_ENUM(Format)

    // Main export methods (callable from QML)
    // sectionIds and motifIds specify which content to export
    // If both are empty, exports everything
    Q_INVOKABLE bool exportToMIDI(const QString &sketchId, const QUrl &filePath,
                                   const QVariantList &sectionIds = QVariantList(),
                                   const QVariantList &motifIds = QVariantList());
    Q_INVOKABLE bool exportToMusicXML(const QString &sketchId, const QUrl &filePath,
                                       const QVariantList &sectionIds = QVariantList(),
                                       const QVariantList &motifIds = QVariantList());
    Q_INVOKABLE bool exportToMSCZ(const QString &sketchId, const QUrl &filePath);

    // Get file extension for format
    Q_INVOKABLE QString fileExtension(int format) const;
    
    // Get suggested filename for sketch
    Q_INVOKABLE QString suggestedFilename(const QString &sketchName, int format) const;

signals:
    void exportStarted(const QString &format);
    void exportProgress(int percent);
    void exportCompleted(const QString &filePath);
    void exportFailed(const QString &error);

private:
    // Internal export implementations
    bool writeMIDIFile(const Sketch &sketch, const QString &filePath,
                       const QStringList &sectionIds, const QStringList &motifIds);
    bool writeMusicXMLFile(const Sketch &sketch, const QString &filePath,
                           const QStringList &sectionIds, const QStringList &motifIds);
    bool writeMSCZFile(const Sketch &sketch, const QString &filePath);

    // Helper methods
    int scaleDegreeToMIDI(int scaleDegree, const QString &key, int octave = 4) const;
    int durationToTicks(const QString &duration, int ticksPerQuarter) const;
    QString keyToMusicXMLFifths(const QString &key) const;
    QString keyToMusicXMLMode(const QString &key) const;

    // MIDI file writing helpers
    void writeMIDIHeader(QDataStream &stream, int numTracks, int ticksPerQuarter);
    void writeMIDITrack(QDataStream &stream, const QByteArray &trackData);
    void writeVariableLengthQuantity(QByteArray &data, int value);

    SketchRepository m_repository;
};

