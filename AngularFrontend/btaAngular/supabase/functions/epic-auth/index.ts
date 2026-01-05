// Follow this setup guide to integrate the Deno language server with your editor:
// https://deno.land/manual/getting_started/setup_your_environment
// This enables autocomplete, go to definition, etc.

// Setup type definitions for built-in Supabase Runtime APIs
import "jsr:@supabase/functions-js/edge-runtime.d.ts"

Deno.serve(async (req) => {
  // Itt kérheted le a titkos kulcsokat, mert ez a szerveren van!
  const clientId = Deno.env.get("EPIC_CLIENT_ID");
  const clientSecret = Deno.env.get("EPIC_CLIENT_SECRET");

  // Itt fogadod az Angular-tól érkező "code"-ot
  const { code } = await req.json();

  // Itt fogsz majd feth-elni az Epic API-ra a tokennért (köv. lépés)
  console.log("Kód megérkezett a backendre:", code);

  return new Response(
    JSON.stringify({ status: "Processing", receivedCode: code }),
    { headers: { "Content-Type": "application/json" } }
  );
})
/* To invoke locally:

  1. Run `supabase start` (see: https://supabase.com/docs/reference/cli/supabase-start)
  2. Make an HTTP request:

  curl -i --location --request POST 'http://127.0.0.1:54321/functions/v1/epic-auth' \
    --header 'Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZS1kZW1vIiwicm9sZSI6ImFub24iLCJleHAiOjE5ODM4MTI5OTZ9.CRXP1A7WOeoJeXxjNni43kdQwgnWNReilDMblYTn_I0' \
    --header 'Content-Type: application/json' \
    --data '{"name":"Functions"}'

*/
