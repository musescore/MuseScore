# FIX ME: temporary muse_framework submodule

This branch points the `muse` submodule at a fork
(`alfonslm/muse_framework`, branch `push-2`) instead of
`musescore/muse_framework`. The audio export changes in this PR need
engine features from musescore/muse_framework#320 (parallel multi-file
export: `IPlayback::saveSoundTracks()`), which aren't upstream yet.
Pointing at the fork keeps this PR buildable in CI until then.

Once musescore/muse_framework#320 is merged:

1. Restore the submodule URL in `.gitmodules` to
   `https://github.com/musescore/muse_framework.git`.
2. Update the `muse` submodule to the merged upstream commit.
3. Delete this file.

The `check_muse_framework` check fails until then, since the submodule
commit isn't in `musescore/muse_framework`.
