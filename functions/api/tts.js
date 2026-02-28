export async function onRequestPost(context) {
    try {
        const { request, env } = context;
        const { text, voiceId } = await request.json();

        // Grab the secret key from Cloudflare
        const apiKey = env.ELEVEN_KEY;

        if (!apiKey) {
            return new Response(JSON.stringify({ error: "ELEVEN_KEY is not defined in Cloudflare settings!" }), { status: 500 });
        }

        const elevenVoice = voiceId || 'Aa6nEBJJMKJwJkCx8VU2'; // Fallback to Bella

        // Call ElevenLabs securely from the backend
        const response = await fetch(`https://api.elevenlabs.io/v1/text-to-speech/${elevenVoice}/stream`, {
            method: 'POST',
            headers: {
                'xi-api-key': apiKey,
                'Content-Type': 'application/json',
                'Accept': 'audio/mpeg'
            },
            body: JSON.stringify({
                text: text,
                model_id: 'eleven_monolingual_v1',
                voice_settings: { stability: 0.45, similarity_boost: 0.80 }
            })
        });

        if (!response.ok) {
            const errData = await response.text();
            throw new Error(`ElevenLabs API error: ${errData}`);
        }

        // Send the raw audio file back to the browser
        return new Response(response.body, {
            headers: {
                'Content-Type': 'audio/mpeg',
                'Access-Control-Allow-Origin': '*'
            }
        });

    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}