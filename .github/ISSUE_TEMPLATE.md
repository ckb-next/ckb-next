Switch on the "Preview" mode for better experience.

---

# You are supposed to:

- [ ] __Read__ the [`README.md`](https://github.com/mattanger/ckb-next/blob/master/README.md).
- [ ] __Skim__ through the other `.md` documents. The chances are they contain [at least] a mention of your problem or [at most] a solution to it.
- [ ] __Search__ before submitting an issue. The chances you will find [at least] a mention of your problem or [at most] a solution to it.
- [ ] __Avoid__ opening a new issue in case a similar one already exists (don't worry, we get the notifications for closed issues as well, it will be reopened if needed).
- [ ] __Keep__ unrelated issues separately (create a new one for a new problem).
- [ ] __Use__ Markdown to write on the GitHub. [Markdown Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet). Use triple backticks if copy-pasting more than _one_ line.

---

# A feature request?

- [ ] yes
- [ ] no

If __yes__, then:

### What is the program's current behavior?

### What would you like it to do instead?

### Any other notes & comments?

---

# A new device support request?

- [ ] yes
- [ ] no

If __yes__, then for
- [ ] __Linux__: upload the output of `ckb-dev-detect` shell script located at the root of the ckb-next source tree
- [ ] __macOS__: upload a full dump following the instructions [here](https://github.com/mattanger/ckb-next/issues/31#issuecomment-285380447)

---

# A bug report?

- [ ] yes
- [ ] no

If __yes__, then for

- [ ] __Linux__:
* distribution's _name_:
* _version_ (pass if rolling):
* _output_ of `uname -r`:

- [ ] __macOS__:
* _version_:

#### Whence?
(how and where did you get this program, e.g.: _"`ckb-next-git` package in AUR"_ or _"built manually using quickinstall script"_ or _"pkg for macOS"_)

#### Version
(ckb-next's version which can be found in the "Settings" tab in the GUI, e.g.: _"ckb-next beta-v0.2.7"_)

#### Branch
(fill in if ckb-next was compiled manually, e.g.: _"master"_ or _"testing"_)

#### Logs & crash reports
(any useful information - an OS crash report, debugger's backtrace, any non-standard way of usage (BIOS mode ...)  etc.)

You should upload:

* a journal log gathered by your init system. Look for __usb__, __ckb__, __ckb-daemon__ and provide a meaningful context
* the output of `ckb-daemon`: `systemctl status ckb-daemon`

### What is the program's current behavior?

### What is the expected behavior of the program?

### Any other notes & comments?

---

# Something else?


