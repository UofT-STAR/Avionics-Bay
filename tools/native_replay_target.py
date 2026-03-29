Import("env")

import os
import subprocess
from SCons.Script import AlwaysBuild


def _run_native_replay(source, target, env):
    exe_path = env.subst("$BUILD_DIR/program.exe")
    if not os.path.exists(exe_path):
        print("Replay executable not found:", exe_path)
        return 1

    sim_env = os.environ.copy()

    # Allow users to override the log path; otherwise use a sensible default.
    if "DATALOG_PATH" not in sim_env:
        default_log = os.path.join(env.subst("$PROJECT_DIR"), "DATALOG.TXT")
        sim_env["DATALOG_PATH"] = default_log

    if "SIM_DATALOG_OUT" not in sim_env:
        sim_env["SIM_DATALOG_OUT"] = os.path.join(env.subst("$PROJECT_DIR"), "SIM_DATALOG.TXT")

    print("Running native replay executable:", exe_path)
    print("DATALOG_PATH=", sim_env["DATALOG_PATH"])
    print("SIM_DATALOG_OUT=", sim_env["SIM_DATALOG_OUT"])

    return subprocess.call([exe_path], env=sim_env)


simulate_target = env.AddCustomTarget(
    name="simulate",
    dependencies=["$BUILD_DIR/program.exe"],
    actions=[_run_native_replay],
    title="Simulate From DATALOG",
    description="Build and run native replay simulation",
)

AlwaysBuild(simulate_target)
