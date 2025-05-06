# MuseScore MacOS QuickLook Preview Provider & Thumbnail Provider

To check if the two plugins are loaded by macOS, run

```bash
pluginkit -m -v | grep muse
```

To view the logs for the plugin, run

```bash
log stream --level info --predicate 'process == "MuseScoreQLPreviewProvider" OR process == "MuseScoreThumbnailProvider"'
```