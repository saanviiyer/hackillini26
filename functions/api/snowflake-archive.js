// Helper to convert a PEM key to a CryptoKey object
async function importPrivateKey(pem) {
    const pemHeader = "-----BEGIN PRIVATE KEY-----";
    const pemFooter = "-----END PRIVATE KEY-----";
    const pemContents = pem.substring(pemHeader.length, pem.length - pemFooter.length).replace(/\s/g, '');
    const binaryDerString = atob(pemContents);
    const binaryDer = new Uint8Array(binaryDerString.length);
    for (let i = 0; i < binaryDerString.length; i++) {
        binaryDer[i] = binaryDerString.charCodeAt(i);
    }
    return crypto.subtle.importKey(
        "pkcs8",
        binaryDer,
        { name: "RSASSA-PKCS1-v1_5", hash: "SHA-256" },
        false,
        ["sign"]
    );
}

async function generateJWT(privateKeyPem) {
    const header = { alg: "RS256", typ: "JWT" };

    // Snowflake account and user details
    const account = "IOSANKQ.HK55137";
    const user = "SAANVISUB";
    const public_key_fp = "SHA256:xC7Y7a8Xe9bRHZIrX/pUtOvLeD7YpdPRleS8uaqf2M0=";

    const qualified_user = `${account}.${user}`;
    const now = Math.floor(Date.now() / 1000);

    const payload = {
        iss: `${qualified_user}.${public_key_fp}`,
        sub: qualified_user,
        iat: now,
        exp: now + 60
    };

    const base64Url = (obj) => btoa(JSON.stringify(obj)).replace(/=/g, "").replace(/\+/g, "-").replace(/\//g, "_");
    const unsignedToken = `${base64Url(header)}.${base64Url(payload)}`;

    const key = await importPrivateKey(privateKeyPem);
    const signature = await crypto.subtle.sign("RSASSA-PKCS1-v1_5", key, new TextEncoder().encode(unsignedToken));
    const base64Signature = btoa(String.fromCharCode(...new Uint8Array(signature))).replace(/=/g, "").replace(/\+/g, "-").replace(/\//g, "_");

    return `${unsignedToken}.${base64Signature}`;
}

export async function onRequestPost(context) {
    const { request, env } = context;

    try {
        const missionData = await request.json();

        // Ensure PRIVATE_KEY exists in Cloudflare Environment Variables
        if (!env.PRIVATE_KEY) {
            throw new Error("PRIVATE_KEY is missing in Cloudflare settings.");
        }

        const token = await generateJWT(env.PRIVATE_KEY);

        // Snowflake API Endpoint
        const url = `https://iosankq-hk55137.snowflakecomputing.com/api/v2/statements`;

        const sqlQuery = `INSERT INTO MISSION_HISTORY (MISSION_ID, MISSION_TYPE, TERRAIN, TILT_DEGREES, AQI_INDEX, TRACTION_SETTING) 
                         VALUES (UUID_STRING(), '${missionData.mission_type}', '${missionData.terrain_classification}', 
                         ${missionData.tilt_degrees}, ${missionData.air_quality_aqi}, '${missionData.traction_setting}')`;

        const response = await fetch(url, {
            method: 'POST',
            headers: {
                "Authorization": `Bearer ${token}`,
                "Content-Type": "application/json",
                "Accept": "application/json",
                "X-Snowflake-Authorization-Token-Type": "KEYPAIR_JWT",
                "User-Agent": "SandozerApp/1.0"
            },
            body: JSON.stringify({
                statement: sqlQuery,
                warehouse: "COMPUTE_WH",
                database: "SANDOZER_DB",
                schema: "PUBLIC"
            })
        });

        const result = await response.json();
        return new Response(JSON.stringify(result), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}