# 🎄🎁 Advent of Code 2022! 🎁🎄
The focus is canonically correct answers; making the correct datastructure in `c` to solve the problem as opposed to hacking my way through. (Until things get to complex then I'll revert to hacking 😅).

| Day | PT1 | PT2 |
| --- | ---- | ---- |
| [1](https://adventofcode.com/2022/day/1) | ⭐️ | ⭐️ |
| [2](https://adventofcode.com/2022/day/2) | ⭐️ | ⭐️ |
| [3](https://adventofcode.com/2022/day/3) | ⭐️ | ⭐️ |
| [4](https://adventofcode.com/2022/day/4) | ⭐️ | ⭐️ |
| [5](https://adventofcode.com/2022/day/5) | ⭐️ | ⭐️ |

## Building 🎅🏻
I am leaning on some things I've built (for reading in a file), which are in the includes folder. This needs to be built first so the solutions can link against it.
```bash
cd ./includes && make
```

Each day is split into `pt1` & `pt2` with a `Makefile` meaning you should be able to run `make` in the folder, run the executable and it should just work™️.