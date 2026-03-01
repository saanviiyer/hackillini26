export async function onRequestPost(context) {
    try {
        const { request, env } = context;
        const missionData = await request.json();

        // Use the namespace name you provided
        const KV = env.MISSION_KV;

        if (!KV) {
            throw new Error("MISSION_KV namespace not bound in Cloudflare Settings.");
        }

        // Generate a unique key for this mission (e.g., mission_17102024...)
        const key = `mission_${Date.now()}`;

        // Save the mission data as a stringified JSON object
        await KV.put(key, JSON.stringify(missionData));

        return new Response(JSON.stringify({ success: true, key }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        console.error('[vector-save] Error:', err.message);
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}