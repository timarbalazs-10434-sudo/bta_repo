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

import cors from 'cors';

// Engedélyezzük az Angularnak, hogy elérje a backendet
app.use(cors({ origin: 'http://localhost:4200', credentials: true }));

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

    if (!code) {
        return res.status(400).send("Missing authorization code");
    }

    try {
        // 1. Token lekérése
        const tokenResponse = await axios.post(
            "https://api.epicgames.dev/epic/oauth/v1/token",
            new URLSearchParams({
                grant_type: "authorization_code",
                code,
                redirect_uri: process.env.EPIC_REDIRECT_URI,
            }),
            {
                headers: {
                    Authorization: "Basic " + Buffer.from(`${process.env.EPIC_CLIENT_ID}:${process.env.EPIC_CLIENT_SECRET}`).toString("base64"),
                    "Content-Type": "application/x-www-form-urlencoded",
                },
            }
        );

        const { access_token } = tokenResponse.data;

        // 2. Felhasználói adatok lekérése (UserInfo)
        const userInfoResponse = await axios.get(
            "https://api.epicgames.dev/epic/oauth/v1/userInfo",
            {
                headers: { "Authorization": `Bearer ${access_token}` }
            }
        );

        // JAVÍTÁS: Itt definiáljuk a változókat, amiket az Angular vár!
        const username = userInfoResponse.data.preferred_username;
        const accountId = userInfoResponse.data.sub;

        console.log("Sikeres bejelentkezés:", username);

        // 3. EGYETLEN válasz küldése a popupnak
        // Ez átadja az adatokat az Angularnak (postMessage) és bezárja a popupot.
        res.send(`
            <script>
                const userData = {
                    username: "${username}",
                    accountId: "${accountId}"
                };
                // Üzenet küldése a főablaknak (Angular)
                window.opener.postMessage({ type: 'AUTH_SUCCESS', data: userData }, "http://localhost:4200");
                // Popup bezárása
                window.close();
            </script>
        `);

    } catch (err) {
        console.error("Epic hiba:", err.response?.data || err.message);
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

// Az Angular ezt fogja hívni
app.get("/api/me", (req, res) => {
    if (req.session.user) {
        res.json(req.session.user);
    } else {
        res.status(401).send("Nincs bejelentkezve");
    }
});

app.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
});
