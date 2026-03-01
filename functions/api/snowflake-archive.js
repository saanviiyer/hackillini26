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

// Helper to generate the Snowflake JWT
async function generateJWT(account, user, privateKeyPem) {
    const header = { alg: "RS256", typ: "JWT" };

    // Snowflake requirements: Account must be uppercase
    const public_key_fp = "SHA256:xC7Y7a8Xe9bRHZIrX/pUtOvLeD7YpdPRleS8uaqf2M0=";
    const full_account = account.toUpperCase();
    const full_user = user.toUpperCase();
    const qualified_user = `${full_account}.${full_user}`;

    const now = Math.floor(Date.now() / 1000);
    const payload = {
        iss: `${qualified_user}.SHA256:${public_key_fp}`,
        sub: qualified_user,
        iat: now,
        exp: now + 60 // Token expires in 60 seconds
    };

    const base64Url = (obj) => btoa(JSON.stringify(obj)).replace(/=/g, "").replace(/\+/g, "-").replace(/\//g, "_");
    const unsignedToken = `${base64Url(header)}.${base64Url(payload)}`;

    const key = await importPrivateKey(privateKeyPem);
    const signature = await crypto.subtle.sign(
        "RSASSA-PKCS1-v1_5",
        key,
        new TextEncoder().encode(unsignedToken)
    );

    const base64Signature = btoa(String.fromCharCode(...new Uint8Array(signature)))
        .replace(/=/g, "").replace(/\+/g, "-").replace(/\//g, "_");

    return `${unsignedToken}.${base64Signature}`;
}

export async function onRequestPost(context) {
    const { request, env } = context;
    const missionData = await request.json();

    try {
        // 1. Generate a fresh token on-the-fly
        // Replace 'YOUR_ACCOUNT' and 'YOUR_USER' with your actual Snowflake IDs
        const token = await generateJWT("HK15537", "SAANVISUB", env.PRIVATE_KEY);

        const url = `https://${env.SNOWFLAKE_ACCOUNT}.snowflakecomputing.com/api/v2/statements`;

        const sqlQuery = `INSERT INTO MISSION_HISTORY (MISSION_TYPE, TERRAIN, TILT_DEGREES, AQI_INDEX, TRACTION_SETTING) 
                         VALUES ('${missionData.mission_type}', '${missionData.terrain_classification}', 
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
        return new Response(JSON.stringify(result), { headers: { "Content-Type": "application/json" } });

    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}