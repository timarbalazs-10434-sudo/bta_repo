import { PrismaClient } from "@prisma/client";
const prisma = new PrismaClient();

export const login = async (req, res) => {
    try 
    {
        const { username, epicAccountId } = req.body;

        if (!username || !epicAccountId) 
        {
            return res.status(400).json({ error: "Missing username or epicAccountId" });
        }
    }
    catch (error) 
    {
        console.error("Login error:", error);
        res.status(500).json({ error: "Internal server error" });
    }
};