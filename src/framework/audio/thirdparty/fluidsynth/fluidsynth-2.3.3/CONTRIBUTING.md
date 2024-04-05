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

### Documentation Style Guide
We use Doxygen for public API functions, usage examples and other
information.

#### Order of Elements
Please ensure that the order of elements in the documentation block
is consistent with the existing documentation. Most importantly,
each function starts with a single sentence brief description,
followed by any `@param` and `@return` tags. `@deprecated` and
`@since` should always come last.

Example:
```
/**
 * Brief description of the function (only a single sentence).
 *
 * @param ...
 * @param ...
 * @return ...
 *
 * Detailed description of the function. This can be multiple paragraphs,
 * include code examples etc. It can also include @note, @warning or
 * other special tags not mentioned below.
 *
 * @deprecated This is deprecated because ...
 *
 * @since 1.2.3
 */
```

#### Mark Lifecycle Functions
All functions are sorted alphabetically in the generated API documentation.
To ensure that the `new_*` and `delete_*` functions appear first, please make
sure to surround those lifecycle functions with `@startlifecycle{}` and
`@endlifecycle` tags.

Example:
```
/** @startlifecycle{Some Name} */
new_fluid_some_name(...);
delete_fluid_some_name(...);
/** @endlifecycle */
```

#### Referencing Setting Items
If you want to mention a setting item (for example `audio.periods`),
please use the custom `@setting{name}` tag. The argument `name` should be
the setting name with all `.` replaced by `_`.

Example:
```
/**
 * This is a comment that references \setting{audio_periods}. You
 * can also link to a group of settings with \setting{audio} or
 * \setting{synth}.
 */
```
