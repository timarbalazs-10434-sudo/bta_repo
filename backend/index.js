import express from "express";
import axios from "axios";
import "dotenv/config";

const app = express();
const PORT = process.env.PORT || 4000;

/**
 * Step A: Redirect user to Epic login
 */
app.get("/auth/epic/login", (req, res) => {
    const clientId = process.env.EPIC_CLIENT_ID;
    const redirectUri = process.env.EPIC_REDIRECT_URI;

    if (!clientId || !redirectUri) 
    {
        console.error("Missing Epic env vars", { clientId, redirectUri });
        return res.status(500).send("Epic configuration missing");
    }

    const authUrl =
        "https://www.epicgames.com/id/authorize" +
        `?client_id=${clientId}` +
        "&response_type=code" +
        "&scope=basic_profile" +
        `&redirect_uri=${encodeURIComponent(redirectUri)}`;

    res.redirect(authUrl);
});


/**
 * Step B: Epic redirects back here with ?code=
 */
app.get("/auth/epic/callback", async (req, res) => {
    const { code } = req.query;

    if (!code) 
    {
        return res.status(400).send("Missing authorization code");
    }

    try {
    const tokenResponse = await axios.post(
        "https://api.epicgames.dev/epic/oauth/v1/token",
        new URLSearchParams({
            grant_type: "authorization_code",
            code,
            redirect_uri: process.env.EPIC_REDIRECT_URI,
        }),
        {
        headers: {
            Authorization:
            "Basic " +
            Buffer.from(
                `${process.env.EPIC_CLIENT_ID}:${process.env.EPIC_CLIENT_SECRET}`
            ).toString("base64"),
            "Content-Type": "application/x-www-form-urlencoded",
        },
        }
    );

    // For now, just return the token
    res.json(tokenResponse.data);
    } catch (err) {
    console.error("Epic token error:");
    console.error(err.response?.status, err.response?.data);
    res.status(500).json({
        error: "Epic authentication failed",
        details: err.response?.data || err.message,
    });
    }
});


app.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
});
