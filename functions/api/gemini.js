export async function onRequestPost(context) {
    try {
        const { request, env } = context;

        // Grab the ENTIRE payload sent from your frontend (history, system prompt, etc.)
        const bodyData = await request.json();

        // Safely grab the API key
        const apiKey = env.GEMINI_KEY || env.GEMINI_API_KEY;

        if (!apiKey) {
            return new Response(JSON.stringify({ error: "⚠ GEMINI_KEY is not defined in Cloudflare settings!" }), { status: 500 });
        }

        // Forward the exact payload to the Gemini API
        const response = await fetch("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(bodyData)
        });

        const data = await response.json();

        return new Response(JSON.stringify(data), {
            headers: { "Content-Type": "application/json" }
        });
    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}