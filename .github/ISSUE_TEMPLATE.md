Switch on the "Preview" mode for better reading experience.
Use it to put ticks and then switch back to write in the details where needed.

<!-- TOC -->

- [Before you procceed](#before-you-procceed)
- [Devices](#devices)
    - [Keyboard](#keyboard)
    - [Mouse](#mouse)
- [New device support request](#new-device-support-request)
- [Feature request](#feature-request)
    - [What is the program's current behavior?](#what-is-the-programs-current-behavior)
    - [What would you like it to do instead?](#what-would-you-like-it-to-do-instead)
    - [Any other notes & comments?](#any-other-notes--comments)
- [Bug report](#bug-report)
    - [General information](#general-information)
        - [Whence?](#whence)
        - [Version](#version)
        - [Branch](#branch)
    - [Logs & crash reports](#logs--crash-reports)
    - [What is the program's current behavior?](#what-is-the-programs-current-behavior-1)
    - [What is the expected behavior of the program?](#what-is-the-expected-behavior-of-the-program)
    - [Any other notes & comments?](#any-other-notes--comments-1)
- [Something else](#something-else)

<!-- /TOC -->

---

# Before you procceed

- [ ] __Try__ the latest code from the [`testing` branch](https://github.com/mattanger/ckb-next/tree/testing).
- [ ] __Read__ the [`README.md`](https://github.com/mattanger/ckb-next/blob/master/README.md) _in full_.
- [ ] __Skim__ through other `.md` documents in the project's root. They contain more specific information.
- [ ] __Search__ in open _and_ closed [Issues](https://github.com/mattanger/ckb-next/issues). [Here's how](https://help.github.com/articles/searching-issues/).
- [ ] __Avoid__ opening a new issue in case a similar one already exists (don't worry, we get the notifications for closed issues as well, it will be reopened if needed).
- [ ] __Keep__ unrelated issues separate (create a new one if you are sure yours is unique).
- [ ] __Use__ Markdown to write on the GitHub. [Markdown Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet). Use triple backticks if copy-pasting more than _one_ line.

Now, __before you proceed even further__, make sure you are following the recommendations above. It will save a lot of time for you and us in the future.

---

# Devices

## Keyboard

<sub><sup>leave empty if you have no keyboard</sup></sub>

* _full name_:
- [ ] RGB
- [ ] non-RGB

## Mouse

<sub><sup>leave empty if you have no mouse</sup></sub>

* _full name_:
- [ ] RGB
- [ ] non-RGB

---

# New device support request

<sub><sup>leave empty if you have no new device support requests</sup></sub>

- [ ] __Linux__: upload the output of `ckb-dev-detect` shell script located at the root of the ckb-next source tree

- [ ] __macOS__: upload a full dump following [the instructions](https://github.com/mattanger/ckb-next/issues/31#issuecomment-285380447)

---

# Feature request

<sub><sup>leave empty if you have no feature requests</sup></sub>

## What is the program's current behavior?

## What would you like it to do instead?

## Any other notes & comments?

---

# Bug report

<sub><sup>leave empty if you have no bug reports</sup></sub>

- [ ] __Linux__:
    * _distribution's name_:
    * _distribution's version_ (pass if rolling):
    * _output_ of `uname -r`:

- [ ] __macOS__:
    * _OS version_:

## General information

### Whence?
(how and where did you get this program, e.g.: _"`ckb-next-git` package in AUR"_ or _"built manually using quickinstall script"_ or _"pkg for macOS"_)

### Version
(ckb-next's version which can be found in the "Settings" tab in the GUI, e.g.: _"ckb-next beta-v0.2.7"_)

### Branch
(fill in if ckb-next was compiled manually, e.g.: _"master"_ or _"testing"_)

## Logs & crash reports
(any useful information - an OS crash report, debugger's backtrace, any non-standard way of usage (BIOS mode ...)  etc.)

You should upload:

- [ ] Linux:
    * a journal log gathered by your init system (`journalctl(1)` etc.) Look for __usb__, __ckb__, __ckb-daemon__ and provide a meaningful context
    * the output of `ckb-daemon`: `systemctl status ckb-daemon`

- [ ] macOS:
    * the output of `ckb-daemon`: `sudo launchctl unload /Library/LaunchDaemons/com.ckb.daemon.plist && sudo /Applications/ckb.app/Contents/Resources/ckb-daemon`

## What is the program's current behavior?

## What is the expected behavior of the program?

## Any other notes & comments?

---

# Something else

<sub><sup>if your issue is about something else, try your best to describe it here, otherwise leave empty</sup></sub>
