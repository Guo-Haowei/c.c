import subprocess

def run_command(args, stdout_path=None):
    """
    Run a command.
    If stdout_path is given, redirect stdout to that file.
    Returns the process return code.
    """
    if stdout_path is not None:
        with open(stdout_path, "w", encoding="utf-8", newline="") as f:
            proc = subprocess.run(args, stdout=f, stderr=subprocess.PIPE, text=True)
    else:
        proc = subprocess.run(args, stderr=subprocess.PIPE, text=True)

    if proc.stderr:
        sys.stderr.write(proc.stderr)

    return proc.returncode
