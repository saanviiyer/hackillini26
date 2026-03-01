export async function onRequestPost(context) {
    try {
        const { request, env } = context;

        // 1. Get the mission data from the robot
        const missionData = await request.json();

        // 2. Create a unique ID for this mission using a timestamp
        const missionId = `mission_${Date.now()}`;

        // 3. Save directly to Cloudflare KV
        // Note: Ensure you named your binding "MISSION_KV" in Cloudflare Settings
        if (!env.MISSION_KV) {
            throw new Error("KV Binding 'MISSION_KV' not found. Check Cloudflare Settings > Functions.");
        }

        await env.MISSION_KV.put(missionId, JSON.stringify(missionData));

        // 4. Return success to the UI
        return new Response(JSON.stringify({
            success: true,
            id: missionId,
            message: "Successfully saved to Cloudflare KV!"
        }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        // This will show up in your Cloudflare "Real-time Logs"
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}