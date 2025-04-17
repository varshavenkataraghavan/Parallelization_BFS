
from fastapi import FastAPI, File, UploadFile, Form
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
import subprocess
import shutil
import os

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/upload")
async def upload_file(file: UploadFile = File(...), mode: str = Form(...)):
    try:
        with open("input.txt", "wb") as f:
            shutil.copyfileobj(file.file, f)

        result = subprocess.run(
            ["./bfs.out", mode],
            capture_output=True,
            text=True,
            timeout=10
        )

        thread_log = ""
        if os.path.exists("web/thread_log.txt"):
            with open("web/thread_log.txt", "r") as log:
                thread_log = log.read()

        return JSONResponse(content={
            "output": result.stdout + result.stderr,
            "thread_log": thread_log
        })

    except Exception as e:
        return JSONResponse(status_code=500, content={"error": str(e)})
