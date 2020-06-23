import subprocess

arguments = ["is_running_checking", "test_terminate", "test_exit_code", "testTL", "test_args", "test_ml",
    "test_re", "test_standard_streams_redirection", "test_il", "test_interactive", "test_exception"]

for arg in arguments:
    if subprocess.run(["test_runner", arg]).returncode == 0:
        print("Test for", arg, "passed")
    else:
        print("Test for", arg, "failed")
        exit(-1)
