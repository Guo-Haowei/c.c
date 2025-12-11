import sys
import difflib
from pathlib import Path

from utils import run_command

# @TODO: refactor error handling and logging
def testcase(c_file: Path) -> bool:
    """
    Run a single test case for the given C file.
    Returns True if passed, False if failed.
    """
    name = c_file.stem
    print(f"Running test [{name}]...")

    # 1) Compile with gcc
    print(f"Compiling {c_file} with gcc")
    ret = run_command(["gcc", str(c_file)])
    if ret != 0:
        print("ERROR: gcc compilation failed.")
        return False

    # 2) Run c.c.exe and capture output -> name.actual
    #    (translation of: .\c.c.exe "%file%" ".\test\%name%.c" > "%name%.actual")
    actual_path = Path(f"{name}.actual")
    test_c_path = Path("test") / f"{name}.c"
    ret = run_command(
        [str(Path("c.c.exe")), str(c_file), str(test_c_path)],
        stdout_path=actual_path,
    )
    if ret != 0:
        print(f"ERROR: c.c.exe failed for {c_file}")
        return False

    # 3) Run a.exe and capture output -> name.expect
    #    (translation of: .\a "..\test\%name%.c" > "%name%.expect")
    #    Using .\test\ instead of ..\test\, since we are in project root.
    expect_path = Path(f"{name}.expect")
    ret = run_command(
        [str(Path("a.exe")), str(test_c_path)],
        stdout_path=expect_path,
    )
    if ret != 0:
        print(f"ERROR: a.exe failed for {c_file}")
        return False

    # 4) Compare outputs (fc /N /W "%name%.expect" "%name%.actual" >nul)
    try:
        actual_text = actual_path.read_text(encoding="utf-8")
        expect_text = expect_path.read_text(encoding="utf-8")
    except OSError as e:
        print(f"ERROR: Failed to read output files for test [{name}]: {e}")
        return False

    RED = "\033[31m"
    GREEN = "\033[32m"
    RESET = "\033[0m"

    if actual_text == expect_text:
        print(f"{GREEN}Test [{name}] passed.{RESET}")
        return True
    else:
        print(f"{RED}ERROR: Test [{name}] failed.{RESET}")

        diff = difflib.unified_diff(
            expect_text.splitlines(keepends=True),
            actual_text.splitlines(keepends=True),
            fromfile=f"{name}.expect",
            tofile=f"{name}.actual",
            lineterm="")

        print("".join(diff))
        return False


def main() -> int:
    # Main loop over .\test\*.c
    test_dir = Path("test")
    if not test_dir.is_dir():
        print("ERROR: test directory 'test' not found.", file=sys.stderr)
        return 1

    c_files = sorted(test_dir.glob("*.c"))
    if not c_files:
        print("No test .c files found in 'test' directory.")
        return 0

    for c_file in c_files:
        if not testcase(c_file):
            # On first failure, mimic `exit /b 1`
            return 1

    # @TODO: make sure to clean up generated even if tests fail
    for pattern in ("*.actual", "*.expect"):
        for file in Path(".").glob(pattern):
            file.unlink()

    return 0


if __name__ == "__main__":
    sys.exit(main())
