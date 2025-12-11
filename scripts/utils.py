import sys
import subprocess
from pathlib import Path

class Log:
    COLOR_RESET = '\033[0m'
    COLOR_GREEN = '\033[32m'
    COLOR_YELLOW = '\033[33m'
    COLOR_RED = '\033[31m'
    COLOR_BOLD_RED = '\033[1;31m'

    @staticmethod
    def info(msg: str):
        print(msg)

    @staticmethod
    def ok(msg: str):
        print(f'{Log.COLOR_GREEN}[ OK ] {msg}{Log.COLOR_RESET}')

    @staticmethod
    def warn(msg: str):
        print(f'{Log.COLOR_YELLOW}[WARN] {msg}{Log.COLOR_RESET}')

    @staticmethod
    def error(msg: str):
        # @TODO: throw exception instead of exiting?
        print(f'{Log.COLOR_RED}[ERR ] {msg}{Log.COLOR_RESET}', file=sys.stderr)

def run_command(args, stdout_path=None):
    '''
    Run a command.
    If stdout_path is given, redirect stdout to that file.
    Returns the process return code.
    '''

    Log.info(f'Running command: {" ".join(args)}')
    if stdout_path is not None:
        with open(stdout_path, 'w', encoding='utf-8', newline='') as f:
            proc = subprocess.run(args, stdout=f, stderr=subprocess.PIPE, text=True)
    else:
        proc = subprocess.run(args, stderr=subprocess.PIPE, text=True)

    if proc.stderr:
        sys.stderr.write(proc.stderr)

    return proc.returncode

def rename(src: Path, dst: Path):
    '''
    Rename a file from src to dst, overwriting dst if it exists.
    '''
    if dst.exists():
        Log.warn(f'Overwriting existing file: {dst}')
        dst.unlink()
    src.rename(dst)
