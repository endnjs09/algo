from fastapi import APIRouter, HTTPException
from app.models.request import VisualizeRequest
from app.middleware.validator import validate_code
from app.services.injector_service import InjectorService
from app.services.sandbox_service import run_sandbox

router   = APIRouter()
injector = InjectorService()


@router.post("/visualize")
async def visualize(request: VisualizeRequest):
    validate_code(request.code, request.stdin)

    try:
        modified_code = injector.inject(request.code)
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Code Analysis Failed: {str(e)}")

    try:
        trace_data = await run_sandbox(modified_code, request.stdin)
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Sandbox execution failed: {str(e)}")

    return trace_data
