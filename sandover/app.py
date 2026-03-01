from fastapi import FastAPI
from pydantic import BaseModel
from google import genai
import uuid
from cortex import CortexClient, DistanceMetric

app = FastAPI()

# ⚠️ IMPORTANT: Paste your actual Google Gemini API Key here
gemini_client = genai.Client(api_key="YOUR_GEMINI_API_KEY")

# Connect to the local Actian Docker container
db_client = CortexClient("localhost:50051")

@app.on_event("startup")
def startup_event():
    # When the server starts, ensure the Actian collection exists!
    if not db_client.has_collection("missions"):
        db_client.create_collection(
            name="missions",
            dimension=768, # Gemini text-embedding-004 uses 768 dimensions
            distance_metric=DistanceMetric.COSINE
        )
        print("✅ Created 'missions' collection in Actian DB!")
    else:
        print("✅ Connected to existing 'missions' collection in Actian DB.")

# Define the data structure matching your frontend
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
async def save_mission(data: MissionData):
    try:
        # 1. Turn the mission data into a highly descriptive paragraph
        mission_text = f"Mission Type: {data.mission_type}. The robot ended at coordinates {data.end_position}. The terrain was classified as {data.terrain_classification} with a tilt of {data.tilt_degrees} degrees. The Air Quality Index (AQI) was {data.air_quality_aqi}. The path taken was: {data.path_taken}."
        
        # 2. Get Embedding Vector from Google's NEW SDK
        response = gemini_client.models.embed_content(
            model="text-embedding-004",
            contents=mission_text
        )
        vector = response.embeddings[0].values
        
        # 3. Save to Actian DB (Requires an integer ID)
        mission_id = int(uuid.uuid4().int % 10000000) 
        
        db_client.upsert(
            collection="missions",
            id=mission_id,
            vector=vector,
            payload=data.dict() # Store the raw JSON data
        )
        
        return {"success": True, "message": "Saved to Actian Vector DB"}
    except Exception as e:
        print("Error saving:", str(e))
        return {"error": str(e)}

@app.post("/search")
async def search_missions(query: SearchQuery):
    try:
        # 1. Get Embedding Vector for the user's question
        response = gemini_client.models.embed_content(
            model="text-embedding-004",
            contents=query.query
        )
        vector = response.embeddings[0].values
        
        # 2. Search Actian DB for the mathematically closest past missions
        results = db_client.search(
            collection="missions",
            query=vector,
            top_k=query.top_k
        )
        
        # 3. Extract the original mission data from the search results
        contexts = [r.payload for r in results]
        
        return {"results": contexts}
    except Exception as e:
        print("Error searching:", str(e))
        return {"error": str(e)}