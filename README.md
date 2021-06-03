# Strobe tuner

Emulating analog [strobe tuner](https://en.wikipedia.org/wiki/Electronic_tuner#Strobe_tuners "Wiki article"). At least trying to do so.
Proof of concept.

## TODO

* Input source selection, for now only default input device is used
* Support for different audio input formats, for now only 32bit floats are supported, which may not work everywhere
* Input threshold value is hardcoded, should be adjustable
* Make target note selectable, for now it works only for bass G string (and it actually does more-or-less)
