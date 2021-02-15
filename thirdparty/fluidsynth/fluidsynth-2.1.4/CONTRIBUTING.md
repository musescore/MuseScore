# Contributing

Thanks for considering to contribute to FluidSynth. Before implementing
any huge new feature, consider bringing up your ideas on our mailing list:
https://lists.nongnu.org/mailman/listinfo/fluid-dev

Contributing can be done by
* [submitting pull requests on Github](
https://help.github.com/articles/proposing-changes-to-your-work-with-pull-requests/) or
* submitting patches to the mailing list.

Patches should be created with `git format-patch`, so in every case you should be familiar with the basics of git. Make sure you develop against the master branch, i.e. **not** against any FluidSynth release.

Some things that will increase the chance that your pull request or patch is accepted:

* Give a reasoning / motivation for any changes or proposals you make.
* Follow our style guide.
* Keep your commits "atomic".
* Write meaningful commit messages.

## Style Guide

Find FluidSynth's style guide below. Syntax related issues, like missing braces, can be taken care of by calling `make format` (provided that cmake has found `astyle` on your system).

#### General
* Every function should have a short comment explaining it's purpose
* Every public API function **must** be documented with purpose, params and return value
* Prefer signed integer types to unsigned ones
* Use spaces rather than tabs
* Avoid macros

#### Naming Conventions
* Words separated by underscores
* Macros always UPPER_CASE
* Function and Variable names always lower_case,  (e.g. `fluid_componentname_purpose()`)

#### Bracing
* Every block after an if, else, while or for should be enclosed in braces
* **Allman-Style** braces everywhere

