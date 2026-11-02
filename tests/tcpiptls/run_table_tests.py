"""Check the legacy remote table test against a loopback VSCP server."""

import contextlib
from pathlib import Path
import subprocess
import sys

from run_transport_tests import Server


def main():
    if len(sys.argv) != 2:
        raise SystemExit("Usage: run_table_tests.py <test_tables_remote_class>")
    executable = Path(sys.argv[1]).resolve(strict=True)

    for reject_noop in (False, True):
        with contextlib.closing(Server(reject_noop=reject_noop)) as server:
            result = subprocess.run(
                [str(executable), server.endpoint(), "admin", "secret"],
                input="\n",
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=15,
            )

        exit_correct = result.returncode > 0 if reject_noop else result.returncode == 0
        error_message = "Error: Failed to run NOOP command."
        if (not exit_correct
                or f"Error Count= {int(reject_noop)}" not in result.stdout
                or (error_message in result.stdout) != reject_noop):
            raise AssertionError(
                f"NOOP rejected={reject_noop}: unexpected exit status {result.returncode} "
                f"or output:\n{result.stdout}"
            )
        print(f"Remote table NOOP {'failure' if reject_noop else 'success'} check passed")


if __name__ == "__main__":
    main()
