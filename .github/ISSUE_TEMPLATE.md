__Switch on the "Preview" mode for better reading experience.__
Use it to put ticks and then switch back to write in the details where needed.

<!-- TOC -->

- [Before you proceed](#before-you-proceed)
- [Devices information](#devices-information)
- [New device support request](#new-device-support-request)
- [Feature request](#feature-request)
    - [What is the program's current behavior?](#what-is-the-programs-current-behavior)
    - [What would you like it to do instead?](#what-would-you-like-it-to-do-instead)
    - [Any other notes & comments?](#any-other-notes--comments)
- [Bug report](#bug-report)
    - [General information](#general-information)
        - [Source](#source)
        - [Branch](#branch)
    - [Logs & crash reports](#logs--crash-reports)
    - [What is the program's current behavior?](#what-is-the-programs-current-behavior-1)
    - [What is the expected behavior of the program?](#what-is-the-expected-behavior-of-the-program)
    - [Any other notes & comments?](#any-other-notes--comments-1)
- [Something else](#something-else)

<!-- /TOC -->

---

# Before you proceed

- [ ] __Try__ the latest code by cloning and building the project manually.
- [ ] __Read__ the [`README.md`](https://github.com/mattanger/ckb-next/blob/master/README.md) _in full_.
- [ ] __Skim__ through other `.md` documents in the project's root. They contain more specific information.
- [ ] __Search__ in open _and_ closed [Issues](https://github.com/mattanger/ckb-next/issues). [Here's how](https://help.github.com/articles/searching-issues/).
- [ ] __Avoid__ opening a new issue in case a similar one already exists (don't worry, we get the notifications for closed issues as well, it will be reopened if needed).
- [ ] __Keep__ unrelated issues separate (create a new one if you are sure yours is unique).
- [ ] __Use__ [Markdown](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet) to write on GitHub. Use triple backticks if copy-pasting more than _one_ line.

Now, __before you proceed even further__, make sure you are following the recommendations above. It will save a lot of time for you and us in the future.

---

# Devices information

Run [`ckb-dev-detect`](https://github.com/mattanger/ckb-next/blob/master/ckb-dev-detect) from the root of the source code tree:

```console
$ ./ckb-dev-detect
```

Upload `ckb-dev-detect-report.gz` on GitHub with this issue.

---

# New device support request

- [ ] Yes
- [ ] No

<sub><sup>leave empty if you have no new device support requests</sup></sub>

You should upload:
* __Linux__: `ckb-dev-detect-report.gz` is enough
* __macOS__: a full dump following [the instructions](https://github.com/mattanger/ckb-next/issues/31#issuecomment-285380447)

---

# Feature request

- [ ] Yes
- [ ] No

<sub><sup>leave empty if you have no feature requests</sup></sub>

## What is the program's current behavior?

## What would you like it to do instead?

## Any other notes & comments?

---

# Bug report

- [ ] Yes
- [ ] No

<sub><sup>leave empty if you have no bug reports</sup></sub>

## General information

### Source
(how and where did you get this program, e.g.: _"`ckb-next-git` package in AUR"_ or _"built manually using quickinstall script"_ or _"pkg for macOS"_)

### Branch
(fill in if ckb-next was compiled manually, e.g.: _"master"_ or _"newdev"_)

## Logs & crash reports
(any useful information - an OS crash report, debugger's backtrace, any non-standard way of usage (BIOS mode ...)  etc.)

You should upload:
- Linux: `ckb-dev-detect-report.gz` is enough
- macOS:
    * the output of `ckb-daemon`:
    ```console
    $ sudo launchctl unload /Library/LaunchDaemons/com.ckb.daemon.plist
    $ sudo /Applications/ckb.app/Contents/Resources/ckb-daemon
    ```
    * the output of `ckb` (open another terminal):
    ```console
    $ sudo killall -9 ckb
    $ /Applications/ckb.app/Contents/MacOS/ckb
    ```

## What is the program's current behavior?

## What is the expected behavior of the program?

## Any other notes & comments?

---

# Something else

<sub><sup>if your issue is about something else, try your best to describe it here, otherwise leave empty</sup></sub>
