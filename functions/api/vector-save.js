export async function onRequestPost(context) {
    try {
        const { request, env } = context;
        const missionData = await request.json();

        const vectorDBUrl = env.ACTIAN_DB_URL;
        const geminiKey = env.GEMINI_KEY || env.GEMINI_API_KEY;

        if (!geminiKey) throw new Error("Missing Gemini Key in Cloudflare!");

        const response = await fetch(`${vectorDBUrl}/insert`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-Gemini-Key': geminiKey,
                'ngrok-skip-browser-warning': 'true' // 👈 ADD THIS LINE
            },
            body: JSON.stringify(missionData)
        });

        if (!response.ok) throw new Error("Failed to save to Vector DB");
        return new Response(JSON.stringify({ success: true }), { headers: { "Content-Type": "application/json" } });
    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}