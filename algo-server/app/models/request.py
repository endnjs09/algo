from pydantic import BaseModel, field_validator

class VisualizeRequest(BaseModel):
    language: str = "cpp"
    code: str
    stdin: str = ""
    client_version: str = "1.0.0"

    @field_validator("language")
    def language_must_be_cpp(cls, v):
        if v.strip().lower() != "cpp":
            raise ValueError("v1.0 only supports C++")
        return v.strip().lower()