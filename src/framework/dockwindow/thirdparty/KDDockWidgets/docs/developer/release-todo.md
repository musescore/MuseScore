# Release Checklist

## Prerelease Source Checking

* make sure the [github actions CI](https://github.com/KDAB/KDDockWidgets/actions) is green
* make sure the [KDAB CI](https://kdab.ci.kdab.com/#/projects/19) is green
* look for any static checking issues (clazy, clang-tidy, cppcheck, etc) that may need fixing.
  check [KDAB CI](https://kdab-artifacts.ci.kdab.com/analysis/kddockwidgets-stable)
  and [Github Nightly CI](https://github.com/KDAB/KDDockWidgets/actions/workflows/nightly.yml)
* look for any changes in the README.md that may need updating or improving.
* follow the [prerelease source code review](prerelease-source.md) instructions.
* merge any changes made during the review
* create the tag (**do not push the tag yet**)

## Prerelease Build Checking

Any last changes before pushing the tag? Any CI failures?

If so:

```shell
    git -d vX.Y.Z # delete the tag; good thing we didn't push it yet
    commit and push fixes
    update Changelog as necessary
    ./deploy/release-kddockwidget.sh X.Y.Z
    git tag -m KDDockWidgets vX.Y.Z X.Y.Z
```

## Release

### Push the tag

```shell
git push --tags
```

### Github

#### Make a release on Github

Go to <https://github.com/KDAB/KDDockWidgets/releases> and make an official release.

* push the "Draft a new release" button.
* use the Changelog to help write the release description.
* hang the kddockwidgets-X.Y.Z.tar.gz, kddockwidgets-X.Y.Z.zip and kddockwidgets-X.Y.0-doc.zip
  (for major and minor releases) on the github release page.
* also hang the associated .asc files on the github release page.

#### Change the default branch

For major (X.Y.0) releases, change the default branch at
<https://github.com/KDAB/KDDockWidgets/settings> to X.Y

## Commercial release

For our paying licensed clients we provide a source code download from our customers.kdab.com portal.

Create a KDADM Jira asking our admins to hang the tar and zips in the customers download area.
The admins should copy kddockwidgets-X.Y.Z.tar.gz, kddockwidgets-X.Y.Z.zip and kddockwidgets-X.Y.0-doc.zip
from <https://github.com/KDAB/KDDockWidgets/releases/vX.Y.Z>.

## Postrelease

### Announcing

* update the
  [Products Release Schedule wiki](https://wiki.kdab.com/display/Products/Product+Release+Schedule)
  and [KDDockWidgets wiki](https://wiki.kdab.com/display/Products/KDDockWidgets)
  with new version numbers

* email the marketing team <marketing@kdab.com> and ask to have the news of
  the KDDockWidgets release posted to KDAB social media.

### Prepare for Next Release

In the branch:

* increment the KDDockWidgets_VERSION_PATCH value in the top-level CMakeLists.txt
* add a new stanza to the top of Changelog for "X.Y.Z+1 (unreleased)"
* merge to the "main" branch
