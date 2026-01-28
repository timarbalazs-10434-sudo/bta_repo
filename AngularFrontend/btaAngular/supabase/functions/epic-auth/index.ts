import { serve } from "https://deno.land/std@0.168.0/http/server.ts"
import { createClient } from 'https://esm.sh/@supabase/supabase-js@2'
import { encode as base64Encode } from "https://deno.land/std@0.82.0/encoding/base64.ts";

const corsHeaders = {
  'Access-Control-Allow-Origin': '*',
  'Access-Control-Allow-Headers': 'authorization, x-client-info, apikey, content-type',
}

// --- KONFIGURÁCIÓ (Írd át a tieidre!) ---
// Ezeket a Supabase Dashboard -> Edge Functions -> Secrets menüben is beállíthatod,
// de most teszteléshez írd be ide keményen (stringként).
const MY_CLIENT_ID = "xyza78914PS090fc0UvUYkrEMtOYpsY0";
const MY_CLIENT_SECRET = "nOIoWeJYhuk7T5YznQjIlw/Tfio66GNl9PdcSdF7QvQ";

serve(async (req) => {
  if (req.method === 'OPTIONS') {
    return new Response('ok', { headers: corsHeaders })
  }

  try {
    const { code } = await req.json()

    // 1. LÉPÉS: TOKEN CSERE JAVÍTÁSA
    const basicAuth = btoa(`${MY_CLIENT_ID}:${MY_CLIENT_SECRET}`);

    const bodyParams = new URLSearchParams();
    bodyParams.append('grant_type', 'authorization_code');
    bodyParams.append('code', code);
    // ADDD HOZZÁ EZT A SORT:
    bodyParams.append('redirect_uri', 'http://localhost:4200/callback');

    const tokenResponse = await fetch('https://api.epicgames.dev/auth/v1/oauth/token', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Authorization': `Basic ${basicAuth}`,
      },
      body: bodyParams.toString(),
    });

    const tokenData = await tokenResponse.json()

    // Szigorú ellenőrzés
    if (!tokenResponse.ok || !tokenData.access_token) {
      console.error("TOKEN HIBA:", tokenData);
      throw new Error(`Epic Token Error: ${JSON.stringify(tokenData)}`);
    }

    // 2. LÉPÉS: USER INFO
    const userResponse = await fetch('https://api.epicgames.dev/auth/v1/accounts/userinfo', {
      headers: { Authorization: `Bearer ${tokenData.access_token}` },
    })

    const userData = await userResponse.json()

    if (!userResponse.ok) {
       console.error("USERINFO HIBA:", userData);
       throw new Error(`Epic UserInfo Error: ${JSON.stringify(userData)}`);
    }

    // 3. LÉPÉS: ADATBÁZIS MENTÉS
    // A Player táblád szerkezete alapján: EpicAccountId, Username, lastLogin
    const supabaseAdmin = createClient(
      Deno.env.get('SUPABASE_URL') ?? '',
      Deno.env.get('SUPABASE_SERVICE_ROLE_KEY') ?? ''
    )

    const { error: dbError } = await supabaseAdmin
      .from('Players') // A te táblád neve
      .upsert({
        EpicAccountId: userData.sub, // Ez a PK
        Username: userData.preferred_username,
        lastLogin: new Date().toISOString()
      })

    if (dbError) {
      console.error("DB HIBA:", dbError);
      throw new Error("Adatbázis hiba: " + dbError.message);
    }

    return new Response(JSON.stringify({ user: userData }), {
      headers: { ...corsHeaders, 'Content-Type': 'application/json' },
      status: 200,
    })

  } catch (error: any) {
    return new Response(JSON.stringify({ error: error.message }), {
      headers: { ...corsHeaders, 'Content-Type': 'application/json' },
      status: 400,
    })
  }
})
