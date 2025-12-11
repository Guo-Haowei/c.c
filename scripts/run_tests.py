import sys
import difflib
from pathlib import Path

from utils import Log, run_command

def testcase(c_file: Path) -> bool:
    '''
    Run a single test case for the given C file.
    Returns True if passed, False if failed.
    '''
    name = c_file.stem
    Log.info(f'Running test [{name}]...')

    # 1. Compile with gcc
    ret = run_command(['gcc', str(c_file)])
    if ret != 0:
        Log.error('ERROR: gcc compilation failed.')
        return False

    # 2. Run c.c.exe and capture output -> name.actual
    actual_path = Path(f'{name}.actual')
    test_c_path = Path('test') / f'{name}.c'
    ret = run_command([str(Path('c.c.exe')), str(c_file), str(test_c_path)], stdout_path=actual_path)

    if ret != 0:
        Log.error(f'c.c failed for {c_file}')
        return False

    # 3. Run a.exe and capture output -> name.expect
    expect_path = Path(f'{name}.expect')
    ret = run_command(
        [str(Path('a.exe')), str(test_c_path)],
        stdout_path=expect_path,
    )
    if ret != 0:
        Log.error(f'a.exe failed for {c_file}')
        return False

    # 4. Compare outputs
    try:
        actual_text = actual_path.read_text(encoding='utf-8')
        expect_text = expect_path.read_text(encoding='utf-8')
    except OSError as e:
        Log.error(f'Failed to read output files for test [{name}]: {e}')
        return False

    if actual_text == expect_text:
        Log.ok(f'Test [{name}] passed.')
        return True
    else:
        diff = difflib.unified_diff(
            expect_text.splitlines(keepends=True),
            actual_text.splitlines(keepends=True),
            fromfile=f'{name}.expect',
            tofile=f'{name}.actual',
            lineterm='')

        Log.error(f'Test [{name}] failed.\n{"".join(diff)}')
        return False

def main() -> int:
    test_dir = 'test'

    c_files = sorted(Path(test_dir).glob('*.c'))
    if not c_files:
        Log.error(f'No test .c files found in "{test_dir}" directory.')
        return 0

    for c_file in c_files:
        if not testcase(c_file):
            return 1

    # @TODO: make sure to clean up generated even if tests fail
    for pattern in ('*.actual', '*.expect'):
        for file in Path('.').glob(pattern):
            file.unlink()

    return 0


if __name__ == '__main__':
    sys.exit(main())
