# Pre-release source code review

See the bash script `deploy/release-kddockwidgets.sh` in this project
which automatically performs all these checks.  Additionally,
`deploy/release-kddockwidgets.sh` does a build of the documentation
plus creates and signs tar/zip balls.  If all goes well, the user
will be prompted to create the tag.

To use it run:

```shell
    ./deploy/release-kddockwidgets.sh X.Y.Z` # where X.Y.Z is the release version.
```

Pre-release checklist:

* sync KDAB cmake files to upstream

  compare the cmake/KDE files to their upstream versions in ssh://codereview.kdab.com:29418/kdab/extra-cmake-modules

* sync KDE cmake files to upstream

  compare the cmake/ECM files to their upstream versions in <https://invent.kde.org/frameworks/extra-cmake-modules>

* ensures the Doxygen.cmake file is up-to-date for the doxygen version we use on KDABCI

    ```shell
    doxygen -u  docs/api/Doxyfile.cmake
    ```

* run a pre-commit check

    ```shell
    pre-commit run --all-files
    ```

* verify ${PROJECT_NAME}_VERSION and ${PROJECT_NAME}_SOVERSION values in the top-level CMakeLists.txt

* Add the real release date to the Changelog section header "vX.Y.Z (unreleased)"

* `git commit` any changes made above and make sure the CIs are green before continuing.

Then create the new tag using the command:

```shell
    git tag -m KDDockWidgets vX.Y.Z X.Y.Z"
```

**DO NOT PUSH THE TAG YET!**
