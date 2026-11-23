"""Optional fix for the Japan build's title text (issue 38), without converting the data again.

In a Japan build the card's title on the race plate and on the title confirm screen can draw some characters (the katakana キ and グ of 連荘キング)
as the red C and S of an English picture: menu_extra.bin's selector discs, top bar titles and 10 out run names sit at picture numbers 110000-110230, and
the Japan titles' glyphs (110000 and up, 1573 of them) share those numbers. This makes a menu_extra.bin with the pictures at 116000 and up instead and puts it
in your built game folder. The player of this version takes whichever the data has, so:

  1. update with setup.bat first (it rebuilds the player, a few minutes);
  2. then run   python tools/setup_patch_titles.py "<your game folder, the one with main and data>"

Nothing else in the built game folder is touched; the old file is kept beside the new one as menu_extra.bin.before-titlefix, and
   python tools/setup_patch_titles.py --undo
puts it back. (This file is not part of what the setup checks to decide whether the data is up to date, so using it or not changes nothing about
future updates; the next update that converts the data again makes these numbers the standard.)
"""
import glob
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

BASES = {"EX_DISC_BASE": 116000, "EX_TITLE_BASE": 116100, "TEN_BGM_BASE": 116200}


def built_folders():
    return sorted(d for d in glob.glob(os.path.join(ROOT, "Super3DXPlus-*")) if os.path.isfile(os.path.join(d, "data", "menu_extra.bin")))


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    undo = "--undo" in sys.argv
    dests = built_folders()
    if len(args) > 1 and os.path.isdir(args[1]):
        dests = [args[1]]
    if not dests:
        sys.exit("No built game folder (Super3DXPlus-JP or -Export, with data/menu_extra.bin) here: run setup.bat first.")
    if undo:
        for d in dests:
            f = os.path.join(d, "data", "menu_extra.bin")
            bak = f + ".before-titlefix"
            if os.path.isfile(bak):
                shutil.copyfile(bak, f)
                print("put back:", f)
            else:
                print("nothing to put back in", d)
        return
    if not args:
        sys.exit(__doc__)
    real = os.path.abspath(args[0])
    if not (os.path.isfile(os.path.join(real, "main")) and os.path.isdir(os.path.join(real, "data"))):
        sys.exit("%s has no main and data/" % real)
    import pipeline
    import extrapack
    view = pipeline._view_dir("arcade-view")
    pipeline.arcade_view(real, view)
    for k, v in BASES.items():  # (extrapack reads them when it runs)
        setattr(extrapack, k, v)
    out = os.path.join(ROOT, "build", "menu_extra.titlefix.bin")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    sys.argv = ["extrapack.py", view, out]
    extrapack.main()
    for d in dests:
        f = os.path.join(d, "data", "menu_extra.bin")
        bak = f + ".before-titlefix"
        if not os.path.isfile(bak):
            shutil.copyfile(f, bak)
        shutil.copyfile(out, f)
        print("patched:", f)
    print("Done. Start the game: the title on the race plate should read in full now.")


if __name__ == "__main__":
    main()
