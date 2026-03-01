export async function onRequestPost(context) {
    const { env, request } = context;

    try {
        const missionData = await request.json();

        // Pull credentials from your Cloudflare environment variables
        const account = env.SNOWFLAKE_ACCOUNT;
        const token = env.SNOWFLAKE_TOKEN;
        const url = `https://${account}.snowflakecomputing.com/api/v2/statements`;

        // Format the SQL query
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

        const response = await fetch(url, {
            method: 'POST',
            headers: {
                "Authorization": `Bearer ${token}`,
                "Content-Type": "application/json",
                "Accept": "application/json",
                "X-Snowflake-Authorization-Token-Type": "bearer"
            },
            body: JSON.stringify({
                statement: sqlQuery,
                warehouse: "COMPUTE_WH",
                database: "SANDOZER_DB",
                schema: "PUBLIC"
            })
        });

        const result = await response.json();

        // Snowflake API returns 200 even for SQL errors, so we check the response code field
        if (result.code && result.code !== '090001') {
            throw new Error(result.message || "Snowflake SQL Execution Error");
        }

        return new Response(JSON.stringify({ success: true }), {
            headers: { "Content-Type": "application/json" }
        });

    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), { status: 500 });
    }
}