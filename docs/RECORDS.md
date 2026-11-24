# Sharing time attack records

The time attack ranking (the machine's runs: the course's top 30, the record time) lives in a folder next to your saves:

    <save folder>/records/

- **Your own runs** are written to `records/my-times.txt` by the game, and only there.
- **Everyone else's** go in the same folder as more files: any name, any number (`alice.txt`, `friends-2026-11.txt`, ...). Every file in the folder is read, and the runs are merged into the ranking the game shows, fastest first. A run that is in two files counts once, so a file renamed or copied does no harm.
- **To share your times,** send `my-times.txt` (rename it as you like: it is only a list of runs). To take someone else's, drop their file in the `records` folder. The game reads the folder when it starts and again within a couple of seconds of a change while you are in the menus, so no restart is needed.
- **To drop someone's times,** take their file out of the folder. Your own runs stay in `my-times.txt`.

What a file holds is one line per run: `rank2 <course> <ms> <hp> <handling grade> <class> <name: 5 characters> <car>`. A line the game cannot place (a car this version does not have, such as a Japan-only or Export-only one, or a number out of range) is skipped, and a file is read up to 2 MB. Your personal bests (YOUR RECORD, per save) stay in the save, as they are yours alone; the course record and the ranking include every file.

The old `ranking` file in the save folder is read the first time (it becomes `records/my-times.txt`), and is still written beside, so a build from before the folder keeps working.

For tests, `MT3DX_RANK_TEST=<ms>` adds a run of that time on course 0 as yours.
