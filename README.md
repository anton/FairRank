FairRank
========

Fair ranking in four player racing competitions.

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
file contains results from races.

### Result file syntax

The result file should contain one race per row, where the nickname of the four
players are listed in order in which they crossed the finish line.
``<Player1> <Player2> <Player3> <Player4>``. Any line in the file
that begins with ``#`` are treated as a comment and will be ignored.

Here follows an example of how a results file could look like.

```
# Example races
Mario Yoshi Bowser Peaches
Yoshi Mario Bowser Peaches
Luigi Mario Bowser Yoshi
# Example comment
TonyRickardsson NickiPedersen TomaszGollob JasonCrump
```

Statistics
----------

When running FairRank statistics will be presented on Standard Output, but also
a file ``data/stats`` will be written that contains the current leaderboard.

