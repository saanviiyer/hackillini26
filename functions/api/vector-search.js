export async function onRequestPost(context) {
    try {
        const { env } = context;

        // Use the namespace name you provided
        const KV = env.MISSION_KV;

        if (!KV) {
            return new Response(JSON.stringify({ context: "KV Namespace MISSION_KV not found." }), {
                headers: { "Content-Type": "application/json" }
            });
        }

        // 1. List the most recent keys
        const list = await KV.list({ limit: 5 });
        const results = [];

        // 2. Loop through and get the mission data
        for (const key of list.keys) {
            const data = await KV.get(key.name, { type: "json" });
            if (data) results.push(data);
        }

        if (results.length === 0) {
            return new Response(JSON.stringify({ context: "No past missions found in database." }), {
                headers: { "Content-Type": "application/json" }
            });
        }

        // 3. Format into a string Gemini can understand
        const contextStr = results.map((r, i) =>
            `Past Mission ${i + 1}: Type: ${r.mission_type}, Terrain: ${r.terrain_classification}, Tilt: ${r.tilt_degrees}°, AQI: ${r.air_quality_aqi}, Time: ${r.timestamp}`
        ).join('\n');

        return new Response(JSON.stringify({ context: contextStr }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        return new Response(JSON.stringify({ context: null, error: err.message }), {
            headers: { "Content-Type": "application/json" }
        });
    }
}