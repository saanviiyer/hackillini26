export async function onRequestPost(context) {
    const { env, request } = context;

    try {
        const missionData = await request.json();

        const account = env.SNOWFLAKE_ACCOUNT;
        const token = env.SNOWFLAKE_TOKEN;
        const url = `https://${account}.snowflakecomputing.com/api/v2/statements`;

        // 🚨 DEBUG LOGGING: Check if environment variables are loading!
        console.log("🚀 [Worker] Sending data to:", url);
        console.log("🔑 [Worker] Token exists?", !!token);

        const sqlQuery = `
      INSERT INTO SANDOZER_DB.PUBLIC.MISSION_HISTORY 
      (MISSION_ID, MISSION_TYPE, TERRAIN, TILT_DEGREES, AQI_INDEX, TRACTION_SETTING)
      VALUES (
        '${crypto.randomUUID()}', 
        '${missionData.mission_type}', 
        '${missionData.terrain_classification}', 
        ${missionData.tilt_degrees}, 
        ${missionData.air_quality_aqi}, 
        '${missionData.traction_setting}'
      );
    `;

        const myHeaders = new Headers();
        myHeaders.append("Authorization", `Bearer ${token}`);
        myHeaders.append("Content-Type", "application/json");
        myHeaders.append("Accept", "application/json");
        myHeaders.append("X-Snowflake-Authorization-Token-Type", "KEYPAIR_JWT");
        myHeaders.append("User-Agent", "SandozerApp/1.0"); // Hard-setting the user agent

        const response = await fetch(url, {
            method: 'POST',
            headers: myHeaders,
            body: JSON.stringify({
                statement: sqlQuery,
                warehouse: "COMPUTE_WH",
                database: "SANDOZER_DB",
                schema: "PUBLIC"
            })
        });

        const result = await response.json();

        // 🚨 DEBUG LOGGING: Print exactly what Snowflake says back
        console.log("❄️ [Snowflake Response]:", JSON.stringify(result, null, 2));

        if (result.code && result.code !== '090001') {
            throw new Error(result.message || "Snowflake SQL Execution Error");
        }

        return new Response(JSON.stringify({ success: true }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        console.error("❌ [Worker Error]:", err.message);
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}