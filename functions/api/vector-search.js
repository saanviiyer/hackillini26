export async function onRequestPost(context) {
    try {
        const { request, env } = context;
        const { query } = await request.json();

        const vectorDBUrl = env.ACTIAN_DB_URL;
        const geminiKey = env.GEMINI_KEY || env.GEMINI_API_KEY;

        if (!geminiKey) throw new Error("Missing Gemini Key in Cloudflare environment variables!");

        // If no Actian URL configured, return graceful fallback
        if (!vectorDBUrl) {
            return new Response(JSON.stringify({
                context: null,
                warning: "ACTIAN_DB_URL not configured. Set it in Cloudflare Settings > Environment Variables."
            }), { headers: { "Content-Type": "application/json" } });
        }

        // Query Actian VectorAI DB via the Python bridge server
        const response = await fetch(`${vectorDBUrl}/search`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-Gemini-Key': geminiKey
            },
            body: JSON.stringify({ query, top_k: 3 })
        });

        if (!response.ok) {
            const errText = await response.text();
            throw new Error(`Actian search failed (${response.status}): ${errText}`);
        }

        const dbData = await response.json();
        const results = dbData.results || [];

        if (!results.length) {
            return new Response(JSON.stringify({
                context: null,
                message: "No past missions found in Actian VectorAI DB yet. Run some missions and save them first!"
            }), { headers: { "Content-Type": "application/json" } });
        }

        // Format results into readable context for the LLM
        const contextStr = results.map((r, i) =>
            `Mission ${i + 1} (similarity: ${(r.score * 100).toFixed(1)}%): ` +
            `Position ${JSON.stringify(r.end_position)}, ` +
            `Terrain: ${r.terrain_classification}, ` +
            `Tilt: ${r.tilt_degrees}°, ` +
            `AQI: ${r.air_quality_aqi}, ` +
            `Path: ${r.path_taken}, ` +
            `Saved: ${r.timestamp}`
        ).join('\n');

        return new Response(JSON.stringify({
            context: contextStr,
            count: results.length
        }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        console.error('[vector-search]', err.message);
        return new Response(JSON.stringify({
            context: null,
            error: err.message
        }), {
            status: 200, // Return 200 so chatbot doesn't hard-fail — just no context
            headers: { "Content-Type": "application/json" }
        });
    }
}