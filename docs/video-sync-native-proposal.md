# Native Video Sync Proposal

## Goal

Add a native reference-video workflow to MuseScore Studio so users can compose against picture, keep video playback aligned with the score playhead, and control video audio through MuseScore playback controls.

## Initial Scope

The first implementation should stay intentionally narrow:

- attach one local video file to a score,
- store video path and offset metadata with the score,
- show a dockable video panel,
- sync play, pause, stop, and seek to MuseScore playback,
- provide video volume and mute controls.

Mixer-level solo and full audio-graph routing should follow after the playback and score-persistence model is accepted.

## User Workflow

1. The user chooses an Attach Video action.
2. MuseScore stores a reference to the selected video and opens a Video panel.
3. When score playback starts, the video starts at the matching score time plus offset.
4. When playback pauses or seeks, the video follows.
5. The user can nudge offset in milliseconds while working.
6. If the video is missing, MuseScore asks the user to relink it.

## Architecture Direction

### Score Metadata

Store score-level attachment data for:

- video file path,
- offset in milliseconds,
- volume,
- mute state,
- future relink identity such as file hash or original file name.

Prefer relative paths when the video is near the score file. Do not embed video data by default.

### Playback Sync

Add a video sync controller that listens to transport state and maps score playback position to video time. The controller should own drift correction rather than putting timing behavior in QML.

### UI

Add a dockable Video panel that uses the existing UI/dock registration patterns. The panel should be able to attach, detach, relink, play/pause through the global transport, and nudge offset.

### Audio

The MVP can begin with video-local mute and volume if needed. The release-quality target is a real Video source in MuseScore's audio/mixer layer so mute and solo behave like other playback channels.

## Open Questions

- Which existing score-project metadata layer is preferred for external media attachments?
- Which playback service should be the source of truth for elapsed score time?
- Is Qt Multimedia acceptable for the video decode/display layer on all supported platforms?
- Should video support be hidden behind an experimental flag while audio routing is incomplete?
- Should video audio participate in audio export, or should export be a later milestone?

## Suggested Patch Sequence

1. Add score/project metadata for an optional video attachment.
2. Add an inert internal video sync model and tests for persistence.
3. Add the Video panel UI.
4. Wire the panel to playback transport.
5. Add audio controls.
6. Add proper mixer/audio routing.
