import os
import json
import logging

from typing import Union

from backend.services.ai_analyzer import load_and_run_model
from fastapi import FastAPI, UploadFile, File

from fastapi.middleware.cors import CORSMiddleware

# logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

app = FastAPI()

origins = [
    "http://localhost:5173", # Your React App URL
    "http://127.0.0.1:5173",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

os.makedirs("uploads", exist_ok=True)

json_file = "files.json"

def save_file(file_location: str, upload_file: UploadFile = File(...)):
    with open(file_location, "wb") as f:
        f.write(upload_file.file.read())
    
    if not os.path.exists(json_file):
        with open(json_file, "w") as f:
            json.dump([], f)

    with open(json_file, "r+") as file:
        try:
            data = json.load(file)
        except json.JSONDecodeError:
            data = []
        
        data.append(upload_file.filename)
        
        file.seek(0)
        json.dump(data, file, indent=4)
        file.truncate()

@app.post("/upload")
async def upload_file(upload_file: UploadFile = File(...)):
    file_location = f"uploads/{upload_file.filename}"
    save_file(file_location, upload_file)

    with open(file_location, 'r') as f:
        content = f.read()
        print(content)

        result = load_and_run_model(content)

        print(result)

    return {"status": "file uploaded", "filename": upload_file.filename}

@app.get("/files")
def get_files():
    with open("files.json", "r") as f:
        files = json.load(f)
    return {"files": files}


@app.get("/files/{file_id}")
def read_item(file_id: int, q: Union[str, None] = None):
    return {"file_id": file_id, "q": q}