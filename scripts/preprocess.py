import subprocess

from pathlib import Path
from utils import Log, run_command, rename

def main():
    # 1. Create c.c with initial lines
    with open('c.c', 'w', encoding='utf-8') as f:
        f.write('#include <stdio.h>\n')
        f.write('#include <stdlib.h>\n')

    # 2. Preprocess c-source.c -> tmp.c
    run_command(['gcc', '-E', '-DPREPROC', 'c-source.c', '-o', 'tmp.c'])

    # 3. Rename tmp.c -> c.c
    rename(Path('tmp.c'), Path('c.c'))
    Log.ok('Preprocessing completed.')

if __name__ == '__main__':
    main()