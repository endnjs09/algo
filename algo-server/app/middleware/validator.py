from fastapi import HTTPException

FORBIDDEN_PATTERNS = [
    "system(", "popen(", "exec(", "fork(",
    "WinExec(", "ShellExecute("
]

MAX_LINES = 500
MAX_CHARS = 20000
MAX_STDIN = 1000

def validate_code(code: str, stdin: str = ""):
    if len(code.splitlines()) > MAX_LINES:
        raise HTTPException(
            status_code = 400,
            detail=f"Code allowed up to {MAX_LINES} lines."
        )

    if len(code) > MAX_CHARS:
        raise HTTPException(
            status_code=400,
            detail=f"Code allowed up to {MAX_CHARS} chars."
        )

    for pattern in FORBIDDEN_PATTERNS:
        if pattern in code:
            raise HTTPException(
                status_code=422,
                detail=f"Contains functions that are not allowed: {pattern}"
            )

    if len(stdin) > MAX_STDIN:
        raise HTTPException(
            status_code=400,
            detail=f"Input values are allowed up to {MAX_STDIN} chars."
        )

