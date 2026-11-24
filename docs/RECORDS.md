# Sharing time attack records

The time attack records live in a folder next to your saves:

    <save folder>/records/

- **Your own runs** are written to `records/my-times.rec` by the game, and only there.
- **Everyone else's** go in the same folder as more files: any name, any number (`alice.rec`, `friends-2026-11.rec`, ...). Every file in the folder is read, and the runs are merged into the machine ranking the game shows (each course's top 30, fastest first) and the course's record time. A run that is in two files counts once, so a file renamed or copied does no harm.
- **To share your times,** send `my-times.rec` (rename it as you like). To take someone else's, drop their file in the `records` folder. The game reads the folder when it starts and again within a couple of seconds of a change while you are in the menus, so no restart is needed.
- **To drop someone's times,** take their file out of the folder. Your own runs stay in `my-times.rec`.
- **Your personal bests** (YOUR RECORD, per course and car) are in `records/card<n>.dat`, one for each save. They are yours alone: they are not merged with anything and are not for sharing.

The files are not text: each is packed and scrambled, with a check of what is in it, so a time cannot be typed in with a text editor. A file that has been changed (or damaged in a bad copy) is left out whole, and the log says so (`records: <file>: not a records file, or changed since it was made`). This is a speed bump, not a lock: the game's code is open, and anyone who builds it can write a file the same way.

A run the game cannot place (a car this version does not have, such as a Japan-only or Export-only one) is skipped, and a file is read up to 2 MB.

The plain files of the builds before this (`ranking`, `records1`, `records2`, ...) are read once, the first time, and become `my-times.rec` and `card<n>.dat`. They are left where they are, unused: an older build still reads them (without the newer runs).

For tests, `MT3DX_RANK_TEST=<ms>` adds a run of that time on course 0 as yours, and `MT3DX_CARD_TEST=<ms>` sets the open save's course 0 best to that time and says what it read.
