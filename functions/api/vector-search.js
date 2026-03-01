export async function onRequestPost(context) {
    try {
        const { request, env } = context;
        const { query } = await request.json();

        const vectorDBUrl = env.ACTIAN_DB_URL;
        const geminiKey = env.GEMINI_KEY || env.GEMINI_API_KEY;

        if (!geminiKey) throw new Error("Missing Gemini Key in Cloudflare!");

        const response = await fetch(`${vectorDBUrl}/search`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-Gemini-Key': geminiKey // 👈 Passing the key to Python!
            },
            body: JSON.stringify({ query: query, top_k: 3 })
        });

        if (!response.ok) throw new Error("Failed to search Vector DB");
        const dbData = await response.json();

        return new Response(JSON.stringify({ context: JSON.stringify(dbData.results) }), {
            headers: { "Content-Type": "application/json" }
        });
    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}