import express from "express";
import axios from "axios";
import "dotenv/config";
import path from "path";

import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const app = express();
const PORT = process.env.PORT || 4000;
app.use(express.static('public'));

// Basic dolgok

app.get("/", (req, res) => {
    // A path.join segít megtalálni a fájlt a projekt mappájában
    res.sendFile(path.join(__dirname, "./index/index.html"));
});

app.get("/profile", (req, res) => {
    res.sendFile(path.join(__dirname, "./app/app.html"));
});

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
        "&prompt=login" +
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

    const { access_token, account_id, preferred_username } = tokenResponse.data;

        console.log("Sikeres token lekérés!");
        console.log("AccountId:", tokenResponse.data.account_id);
        console.log("token:", tokenResponse.data.access_token);

const userInfoResponse = await axios.get(
    "https://api.epicgames.dev/epic/oauth/v1/userInfo",
    {
        headers: { 
            "Authorization": `Bearer ${access_token}` 
        }
    }
);

    const username = userInfoResponse.data.preferred_username;
    const accountId = userInfoResponse.data.sub;
    
    res.redirect(`../../profile?username=${encodeURIComponent(username)}&accountId=${accountId}`);

    
    } catch (err) {
    console.error("Epic token error:");
    console.error(err.response?.status, err.response?.data);
    res.status(500).json({
        error: "Epic authentication failed",
        details: err.response?.data || err.message,
    });
    }
});

app.get("/full-logout", (req, res) => {
    // Az Epic logout URL-je
    const epicLogoutUrl = "https://www.epicgames.com/id/logout?redirect_uri=" + encodeURIComponent("http://localhost:4000");
    res.redirect(epicLogoutUrl);
});


app.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
});
