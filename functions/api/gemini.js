export async function onRequestPost(context) {
    try {
        // context.env holds your secret keys
        // context.request holds the data sent from your frontend
        const { request, env } = context;
        const { message } = await request.json();

        // Call the Gemini API using the hidden key
        const response = await fetch("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + env.GEMINI_API_KEY, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ contents: [{ parts: [{ text: message }] }] })
        });

        const data = await response.json();

        // Send the Gemini response back to your frontend
        return new Response(JSON.stringify(data), {
            headers: { "Content-Type": "application/json" }
        });
    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}