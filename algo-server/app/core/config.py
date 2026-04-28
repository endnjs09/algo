from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    GGUF_SERVER_URL: str = "http://localhost:8080"
    SANDBOX_IMAGE: str = "algo-sandbox"
    SANDBOX_TIMEOUT: int = 10

    class Config:
        env_file = ".env"

settings = Settings()