import subprocess
from pathlib import Path

from utils import run_command, Log

def rename(src: Path, dst: Path):
    """Rename a file from src to dst, overwriting dst if it exists."""
    if dst.exists():
        dst.unlink()
    src.rename(dst)

def main():
    # 1. Create c.c with initial lines
    with open('c.c', 'w', encoding='utf-8') as f:
        f.write('#include <stdio.h>\n')
        f.write('#include <stdlib.h>\n')

    # 2. Preprocess c-source.c -> tmp.c
    run_command(["gcc", "-E", "-DPREPROC", "c-source.c", "-o", "tmp.c"])

    # # 3. Read tmp.c, filter out # lines and blank lines, append to c.c
    # with open("tmp.c", "r", encoding="utf-8") as src, \
    #      open("c.c", "a", encoding="utf-8") as dst:
    #     for line in src:
    #         stripped = line.strip()
    #         if not stripped:
    #             continue        # skip blank lines
    #         if stripped.startswith("#"):
    #             continue        # skip preprocessor lines
    #         dst.write(line)

    # 4. Rename tmp.c -> c.c
    rename(Path('tmp.c'), Path('c.c'))
    Log.ok('Preprocessing completed.')

if __name__ == '__main__':
    main()