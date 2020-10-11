This is a version of tintin++ 1.99.6 that I forked back in the late 2000s to
include some features in which I was interested.  When I returned to mudding in
2020, I discovered that the current version of tintin++ was unstable and not to
my liking, so I dig up this old fork and redid the autoconf to compile on both
Linux and MacOS.

I don't really remember what's different other than the `#read` command will
look for files suffixed with `.script` in addition to the literal file it
receives so that you can, for example, `#read mychar` to load `mychar.script`.

[![Build Status](https://travis-ci.org/glennklockwood/tintin.svg?branch=main)](https://travis-ci.org/glennklockwood/tintin)
