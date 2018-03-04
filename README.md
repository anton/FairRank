FairRank
========

Fair ranking in score based head-to-head competitions.

Compiling
---------

If ``llvm`` is installed with ``clang++`` it should be enough to use the
accompanied ``Makefile`` to be able to compile the program.

```
make
````

Results file
------------

FairRank read a results file that should be stored in ``data/results``. The
file contains results from games.

### Result file syntax

The result file should contain one game per row, where the nickname of the two
players are listed along with the scores they achived in the game.
``<Player1> <Player2> <Player1Score>-<Player2Score>``. Any line in the file
that begins with ``#`` are treated as a comment and will be ignored.

Here follows an example of how a results file could look like.

```
# Example match
Federer    Edberg    2-0
Nickname1  Nickname2 3-3
Federer    Anton     0-5
# Example comment
Edberg     Nickname2 1-0
```

Statistics
----------

When running FairRank statistics will be presented on Standard Output, but also
a file ``data/stats`` will be written that contains the current leaderboard.

Dynamic library
---------------

See [sample.c](sample.c) for an example of how to use libfairrank.so.
