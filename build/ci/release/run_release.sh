name: Release

on:
  push:
    branches:
      - master

jobs:
  release:
    if: startsWith(github.ref, 'refs/tags/v')
    runs-on: ubuntu-latest
    steps:
      - name: Checkout Repository
        uses: actions/checkout@main
        with:
          token: ${{ secrets.PERSONAL_ACCESS_TOKEN }}
          fetch-depth: 0
      - name: Setup Python
        uses: actions/setup-python@v2.1.4
        with:
          python-version: 3.8.6
          architecture: x64
      - name: Get version from tag
        id: tag_name
        run: |
          echo ::set-output name=current_version::${GITHUB_REF#refs/tags/v}
        shell: bash
      - name: Get notes
        id: generate_notes
        uses: anmarkoulis/commitizen-changelog-reader@master
        with:
          tag_name: ${{ github.ref }}
          changelog: CHANGELOG.md
      - name: Create Release
        id: create_release
        uses: actions/create-release@v1
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
        with:
          tag_name: ${{ github.ref }}
          release_name: Release ${{ github.ref }}
          prerelease: false
          draft: false
          body: ${{join(fromJson(steps.generate_notes.outputs.notes).notes, '')}}
