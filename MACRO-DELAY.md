ckb: RGB Driver for Linux and OS X with Macro Delay
===================================================

This `macro-delay` version of `ckb` implements global and local delays during macro action playback.  Setting a _global delay_ value introduces a time delay between events during macro execution or playback. _Local delay_ allows setting the delay after an individual event, overriding the global delay value for that event. Thus global delay can be used to set the overall playback speed of macros and local delays can be used to tune individual events within a macro.

All delay values are specified in microseconds (us) and are positive values from `0` to  `UINT_MAX - 1`. This means delays range from 0 to just over 1 hour (4,294,967,294us, 4,294 seconds, 71 minutes, or 1.19 hours). A value of zero (0) represents no delay between actions.

Global Delay
------------

Global delay allows macro playback speed to be changed. It sets the time between (actually after) each recorded macro event. If global delay is set to 1 microsecond then a 1 ms delay will follow each individual macro event when the macro is triggered.

The _global delay_ is set with the ckb-daemon's existing (in testing branch) `delay` command followed by an unsigned integer representing the number of microseconds to wait after each macro action and before the next.

Global delay can also be set to `on` which maintains backwards compatibility with the current development of `ckb-daemon` for long macro playback. That is, setting the global delay to `on` introduces a 30us and a 100us delay based on the macro's length during playback.

**NOTE**: This setting also introduces a delay after the last macro action. This functionality exists in the current testing branch and was left as-is. It is still to be determined if this is a bug or a feature.

*Examples*:
````
    delay 1000  # delay 1,000us between action playback
    delay on    # long macro delay -- how it worked before:
                #   30us for 'short' macros (<200 actions)
                #   100us for 'long' macros (>=200 acitions)
    delay off   # no delay (same as 0)
    delay 0     # no delay (same as off)
    delay spearmint-potato    # invalid input, no delay (same as off)
````

Local Delay
-----------

Local Delay allows each macro action to have a post-action delay associated with it. This allows a macro to vary it's playback speed for each event. If no local delay is specified for a macro action, then the global `delay` (above) is used. All delay values are in microsecods (us) as with the global delay setting.

*Example*: define a macro for `g5` with a 5,000us delay between the `e`  down and `e` up actions. A 1,000us delay between `l` up and `a` down, a delay of one second (1,000,000us) after `y` up and before `enter`, and the global delay for all other actions.

````
    macro g5:+d,-d,+e=5000,-e,+l,-l=10000,+a,-a,+y,-y=1000000,+enter,-enter
````

*Example*: use default delay between `d` down and `d` up and no delay (0us) after `d` up. This removes the noted feature/bug (above) where the last action has a trailing delay associated with it.

````
    macro g5:+d,-d=0
````
