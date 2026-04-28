from pydantic import BaseModel
from typing import Any

class VisualizeResponse(BaseModel):
    traces: Any