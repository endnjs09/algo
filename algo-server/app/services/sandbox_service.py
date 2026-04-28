import json
import os
import tempfile
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor
from fastapi import HTTPException
from app.core.config import settings
import logging

logger = logging.getLogger(__name__)

# SANDBOX_IMAGE = settings.SANDBOX_IMAGE
SANDBOX_IMAGE = "algo-visual-sandbox:latest"
TIMEOUT = settings.SANDBOX_TIMEOUT
executor = ThreadPoolExecutor()

def _convert_path(path: str) -> str:
    if len(path) > 1 and path[1] == ':':
        drive = path[0].lower()
        rest = path[2:].replace('\\', '/')
        return f"/{drive}{rest}"
    return path

def _run_docker(tmpdir: str, stdin: str) -> subprocess.CompletedProcess:
    docker_path = _convert_path(tmpdir)

    # Escape stdin to avoid shell injection through echo
    safe_stdin = stdin.replace("'", "'\\''")

    docker_cmd = [
        "docker", "run", "--rm",
        "--network", "none",
        "--memory", "256m",
        "--cpus", "1.0",
        "-v", f"{docker_path}:/work",
        SANDBOX_IMAGE,
        "bash", "-c",
        "cp /sandbox/TraceLogger.h /work/ && "
        "cp /sandbox/TraceLogger.cpp /work/ && "
        "cp -r /sandbox/shadow_include /work/ && "
        "cd /work && "
        "g++ -std=c++17 -I. -I./shadow_include -o temp_runner temp_runner.cpp TraceLogger.cpp && "
        f"echo '{safe_stdin}' | ./temp_runner"
    ]

    result = subprocess.run(
        docker_cmd,
        capture_output=True,
        text=True,
        timeout=TIMEOUT
    )
    logger.debug(f"returncode: {result.returncode}")
    logger.debug(f"stdout: {result.stdout}")
    logger.debug(f"stderr: {result.stderr}")
    return result

async def run_sandbox(modified_code: str, stdin: str = "") -> dict:
    import asyncio
    tmpdir = tempfile.mkdtemp()

    try:
        cpp_path = os.path.join(tmpdir, "temp_runner.cpp")
        with open(cpp_path, "w", encoding="utf-8") as f:
            f.write(modified_code)

        loop = asyncio.get_event_loop()
        try:
            result = await loop.run_in_executor(
                executor, _run_docker, tmpdir, stdin
            )
        except subprocess.TimeoutExpired:
            raise HTTPException(status_code=408, detail="Running Timeout (10 sec)")

        if result.returncode != 0:
            raise HTTPException(
                status_code=422,
                detail=f"Compile Error: {result.stderr}"
            )

        trace_path = os.path.join(tmpdir, "trace.json")
        if not os.path.exists(trace_path):
            raise HTTPException(status_code=500, detail="Failed to create trace.json")

        with open(trace_path, "r", encoding="utf-8") as f:
            trace_data = json.load(f)

        return trace_data

    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)