import os
import json

from typing import Union

from fastapi import FastAPI, UploadFile, File

from fastapi.middleware.cors import CORSMiddleware

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

# Ensure directories exist on startup or before save
os.makedirs("uploads", exist_ok=True)

json_file = "files.json"

@app.post("/upload")
def upload_file(upload_file: UploadFile = File(...)):
    file_location = f"uploads/{upload_file.filename}"
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
        
    return {"status": "file uploaded", "filename": upload_file.filename}

@app.get("/files")
def get_files():
    with open("files.json", "r") as f:
        files = json.load(f)
    return {"files": files}


# @app.get("/items/{item_id}")
# def read_item(item_id: int, q: Union[str, None] = None):
#     return {"item_id": item_id, "q": q}