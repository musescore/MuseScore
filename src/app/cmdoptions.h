#ifndef MU_APP_CMDOPTIONS_H
#define MU_APP_CMDOPTIONS_H

#include <optional>
#include <string>

#include "global/io/path.h"
#include "global/iapplication.h"
#include "global/logger.h"

namespace mu::app {
enum class ConvertType {
    File,
    Batch,
    ConvertScoreParts,
    ExportScoreMedia,
    ExportScoreMeta,
    ExportScoreParts,
    ExportScorePartsPdf,
    ExportScoreTranspose,
    SourceUpdate,
    ExportScoreVideo
};

enum class DiagnosticType {
    Undefined = 0,
    GenDrawData,
    ComDrawData,
    DrawDataToPng,
    DrawDiffToPng
};

struct CmdOptions {
    enum class ParamKey {
        HighlightConfigPath,
        StylePath,
        ScoreSource,
        ScoreTransposeOptions,
        ForceMode,
        SoundProfile,
        ExtensionUri

        // Video
    };

    muse::IApplication::RunMode runMode = muse::IApplication::RunMode::GuiApp;

    struct {
        std::optional<double> physicalDotsPerInch;
    } ui;

    struct {
        std::optional<bool> templateModeEnabled;
        std::optional<bool> testModeEnabled;
    } notation;

    struct {
        std::optional<bool> fullMigration;
    } project;

    struct {
        std::optional<int> trimMarginPixelSize;
        std::optional<float> pngDpiResolution;
    } exportImage;

    struct {
        std::optional<int> mp3Bitrate;
    } exportAudio;

    struct {
        std::optional<std::string> resolution;
        std::optional<int> fps;
        std::optional<double> leadingSec;
        std::optional<double> trailingSec;
    } exportVideo;

    struct {
        std::optional<muse::io::path_t> operationsFile;
    } importMidi;

    struct {
        std::optional<bool> useDefaultFont;
        std::optional<bool> inferTextType;
    } importMusicXml;

    struct {
        std::optional<bool> linkedTabStaffCreated;
        std::optional<bool> experimental;
    } guitarPro;

    struct {
        std::optional<bool> revertToFactorySettings;
        std::optional<muse::logger::Level> loggerLevel;
    } app;

    struct {
        std::optional<std::string> type;
        std::optional<QUrl> scoreUrl;
        std::optional<QString> scoreDisplayNameOverride;
    } startup;

    struct ConverterTask {
        ConvertType type = ConvertType::File;

        QString inputFile;
        QString outputFile;

        QMap<ParamKey, QVariant> params;
    } converterTask;

    struct Diagnostic {
        DiagnosticType type = DiagnosticType::Undefined;
        QStringList input;
        QString output;
    } diagnostic;

    struct Autobot {
        QString testCaseNameOrFile;
        QString testCaseContextNameOrFile;
        QString testCaseContextValue;
        QString testCaseFunc;
        QString testCaseFuncArgs;
    } autobot;

    struct AudioPluginRegistration {
        muse::io::path_t pluginPath;
        bool failedPlugin = false;
        int failCode = 0;
    } audioPluginRegistration;
};
}

#endif // MU_APP_CMDOPTIONS_H
