export async function onRequestPost(context) {
    try {
        const { request, env } = context;
        const missionData = await request.json();
        const missionId = `mission_${Date.now()}`;
        const results = { cloudflare: null, actian: null };

        // ── 1. Cloudflare KV Save ──────────────────────────────────
        if (env.MISSION_KV) {
            await env.MISSION_KV.put(missionId, JSON.stringify(missionData));
            results.cloudflare = { success: true, id: missionId };
        } else {
            results.cloudflare = { success: false, error: "KV Binding 'MISSION_KV' not found." };
        }

        // ── 2. Actian VectorAI DB Save ─────────────────────────────
        const actianUrl = env.ACTIAN_DB_URL;
        if (actianUrl) {
            try {
                const actianRes = await fetch(`${actianUrl}/save`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(missionData)
                });

                if (actianRes.ok) {
                    const actianData = await actianRes.json();
                    results.actian = { success: true, id: actianData.id, text: actianData.text };
                } else {
                    const errText = await actianRes.text();
                    results.actian = { success: false, error: `Actian server error: ${actianRes.status} — ${errText}` };
                }
            } catch (actianErr) {
                results.actian = { success: false, error: `Actian unreachable: ${actianErr.message}` };
            }
        } else {
            results.actian = { success: false, error: "ACTIAN_DB_URL not set in Cloudflare environment variables." };
        }

        const overallSuccess = results.cloudflare?.success || results.actian?.success;

        return new Response(JSON.stringify({
            success: overallSuccess,
            id: missionId,
            message: `Saved to: ${[
                results.cloudflare?.success ? 'Cloudflare KV' : null,
                results.actian?.success ? 'Actian VectorAI DB' : null
            ].filter(Boolean).join(' + ') || 'nowhere (check env vars)'}`,
            details: results
        }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}