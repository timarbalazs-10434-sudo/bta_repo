const clientId = "xyza78914PS090fc0UvUYkrEMtOYpsY0";
const clientSecret = "nOIoWeJYhuk7T5YznQjIlw/Tfio66GNl9PdcSdF7QvQ";
const redirectUri = "http://localhost:4200/callback";

const corsHeaders = {
  'Access-Control-Allow-Origin': '*',
  'Access-Control-Allow-Headers': 'authorization, x-client-info, apikey, content-type',
  'Access-Control-Allow-Methods': 'POST, OPTIONS',
};

Deno.serve(async (req) => {
  // CORS kezelés (maradjon benne!)
  if (req.method === 'OPTIONS') {
    return new Response('ok', { headers: corsHeaders });
  }

  try {
    const { code } = await req.json();

    // 1. LÉPÉS: Code kicserélése Tokenre
    const tokenResponse = await fetch("https://api.epicgames.dev/epic/oauth/v1/token", {
      method: "POST",
      headers: {
        "Content-Type": "application/x-www-form-urlencoded",
        "Authorization": `Basic ${btoa(`${clientId}:${clientSecret}`)}`,
      },
      body: new URLSearchParams({
        grant_type: "authorization_code",
        code: code,
        redirect_uri: redirectUri,
        scope: "basic_profile",
      }),
    });

    const tokenData = await tokenResponse.json();

    if (!tokenData.access_token) {
      throw new Error("Nem sikerült access tokent szerezni az Epictől.");
    }

    // 2. LÉPÉS: Profiladatok lekérése a kapott tokennel
    const userResponse = await fetch("https://api.epicgames.dev/epic/oauth/v1/userInfo", {
      headers: {
        "Authorization": `Bearer ${tokenData.access_token}`,
      },
    });

    const userInfo = await userResponse.json();

    // 3. VÁLASZ: Küldjük vissza a valódi adatokat az Angularnak
    return new Response(
      JSON.stringify({
        username: userInfo.preferred_username,
        epicAccountId: userInfo.sub
      }),
      { headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );

  } catch (error) {
    return new Response(
      JSON.stringify({ error: error.message }),
      { status: 400, headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );
  }
})
