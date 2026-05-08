# The muse framework is now a submodule

We are separating the Muse Framework into a separate repository, essentially a separate project.
The main reason is that it is currently used in two applications - MuseScore and Audacity!
At the moment (for the last 2 years), Audacity has been using a temporary copy of the framework, which it periodically synchronizes.

But the time has come for us to be on equal terms. Audacity want to contribute to the framework's development. To achieve this, we want to see the framework as a separate entity, not part of MuseScore, which both teams develop equally.

We've been working towards this for a long time, and we've been preparing the framework for this event step by step.

This is a huge change for MuseScore!! It will impact all our development processes. It will be challenging for us to adapt.
But I believe we can handle it!

# New Way 

*This isn't a guide to how we'll work from now on, what processes we should have, etc.
It's more a description of the experience we're currently gaining.
After we gain enough experience, we'll describe the process.*

## Local development 

We're in the process of transitioning to a new development process...
Much will be decided as we go along this way.

What I can recommend now
The basic idea is to use your own fork of MF if you need to make changes in both repositories, or for example only in MF, but you would like to compile the application and test it.
Then you can make simultaneous changes, commit them, and simultaneously make two PRs on both projects.

There are at least two approaches to this:

1. Switch the submodule to use its own fork of the MF repository.
It's best to ask the AI ​​how to do this more easily.
The advantage of this approach is that you work with a single repository and can see all changes, which can be especially convenient if you use a Git client.
The disadvantage is that you can accidentally push use your own fork to upstream; you need to monitor this.
We have a CI check, we will see if the submodule is not original.

2. Use a clone of your forked MF repository and specify the path to it.
You can do this directly in the project tree for convenience.
Clone your forked MF
Rename it so that it ends with `.local` for example, `muse.local`, `mf.local` - this will cause this folder to be ignored by the main repository.
Specify the path to this folder in the cmake options. MUSE_FRAMEWORK_PATH=...
The project will be built from two separate repositories. You can work with each of them using the usual workflow. Another advantage is that you don't have to change any project settings and can't accidentally push anything to upstream.
The disadvantage is that you have to manually change and track which changes are which, which active branch, and so on.

## Updating the MF submodule

Problem: The submodule may not be updated or point to a different fork.

Typical situation:
* During development, the developer replaced the submodule with his own fork and forgot to change it back.
* Some PR depends on changes in the MF. They were made and merged into the MF, but the submodule was forgotten or didn't know it needed to be updated. Although the test should have detected this.
* Some fixes have been made in the MF, but no one has updated the submodule in the MU.

The idea is that any PR will require the submodule to point to the latest commit, so the developer creating the PR will be forced to update the submodule. This is similar to the requirement to now fetch the latest upstream changes before creating a PR, or keeping the master always in working order. 

That is, we now treat updating the MF submodule not as updating dependent changes in some PR, but as keeping the code always up-to-date.
There are too many of us to keep track of whether a submodule can be updated or not. Therefore, we are currently adopting the strategy of "the submodule can and should always be updated; it must always be up-to-date."

Therefore, we added a check to the CI that checks the submodule's up-to-date in each PR.
If the check fails, the submodule needs to be updated: `git submodule update --init --remote ./muse` 

## Make PR and build testing

We have little experience here. But it looks like this:
* A contributor (external or internal) forks the program and framework
* makes changes and pushes to new branches
* creates a PR in the program's repo, pointing to their fork of the framework (we'll see that the submodule check fails, we can't merge it)
* everything is reviewed there, including framework changes
* after approval, a PR is created for the framework
* the framework is merged
* the framework link is updated in the program's PR
* the program is merged 

but there are many different cases here 

### Cases 

1. The changes to the MF are not breaking (for example, they added a new field), but the APP requires them (it does not work without this field)

In this case, you need to make the PR in the APP a draft.
Merge the PR into the MF
Update the PR in the APP and make it ready for review

2. Changes in MF are breaking (for example, they removed something), changes in APP are not breaking (they removed the use of something from MF) 

In this case, you need to make the PR in the MF a draft.
First, merge the PR into the APP.
Then make the PR in the MF ready for review and merge.

So, the general approach is:
* Anything that breaks or is broken - we make a draft
* Anything that doesn't break (either expands or, conversely, removes use) - we merge first 

3. The changes in the MF are breaking, but the APP requires these changes.

This is a complex case.
A good solution is to make a case 1 or 2, i.e., add some defines or duplicate the code... We've had experience with very large changes that occurred evolutionarily (updating Qt, adding contexts).

If everything is difficult, we'll consider these cases separately. For example, stop everything, merge both PRs, move on.
