from fastapi import FastAPI, Header, HTTPException
from pydantic import BaseModel
from google import genai
import uuid
from cortex import CortexClient, DistanceMetric

app = FastAPI()

# Connect to the local Actian Docker container
db_client = CortexClient("localhost:50051")

@app.on_event("startup")
def startup_event():
    if not db_client.has_collection("missions"):
        db_client.create_collection(
            name="missions",
            dimension=768, 
            distance_metric=DistanceMetric.COSINE
        )
        print("✅ Created 'missions' collection in Actian DB!")
    else:
        print("✅ Connected to existing 'missions' collection in Actian DB.")

class MissionData(BaseModel):
    mission_type: str
    end_position: list
    terrain_classification: str
    tilt_degrees: float
    air_quality_aqi: int
    path_taken: str

class SearchQuery(BaseModel):
    query: str
    top_k: int = 3

@app.post("/insert")
async def save_mission(data: MissionData, x_gemini_key: str = Header(None)):
    if not x_gemini_key:
        raise HTTPException(status_code=401, detail="Missing Gemini Key from Cloudflare")
        
    try:
        # Initialize Gemini using the key passed from Cloudflare
        gemini_client = genai.Client(api_key=x_gemini_key)
        
        mission_text = f"Mission Type: {data.mission_type}. The robot ended at coordinates {data.end_position}. The terrain was classified as {data.terrain_classification} with a tilt of {data.tilt_degrees} degrees. The Air Quality Index (AQI) was {data.air_quality_aqi}. The path taken was: {data.path_taken}."
        
        response = gemini_client.models.embed_content(
            model="text-embedding-004",
            contents=mission_text
        )
        vector = response.embeddings[0].values
        
        mission_id = int(uuid.uuid4().int % 10000000) 
        
        db_client.upsert(
            collection="missions",
            id=mission_id,
            vector=vector,
            payload=data.model_dump() # Updated for newer Pydantic versions
        )
        
        return {"success": True, "message": "Saved to Actian Vector DB"}
    except Exception as e:
        print("Error saving:", str(e))
        return {"error": str(e)}

@app.post("/search")
async def search_missions(query: SearchQuery, x_gemini_key: str = Header(None)):
    if not x_gemini_key:
        raise HTTPException(status_code=401, detail="Missing Gemini Key from Cloudflare")
        
    try:
        # Initialize Gemini using the key passed from Cloudflare
        gemini_client = genai.Client(api_key=x_gemini_key)
        
        response = gemini_client.models.embed_content(
            model="text-embedding-004",
            contents=query.query
        )
        vector = response.embeddings[0].values
        
        results = db_client.search(
            collection="missions",
            query=vector,
            top_k=query.top_k
        )
        
        contexts = [r.payload for r in results]
        return {"results": contexts}
    except Exception as e:
        print("Error searching:", str(e))
        return {"error": str(e)}