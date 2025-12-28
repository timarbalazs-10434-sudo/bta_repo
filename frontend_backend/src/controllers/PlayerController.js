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

        const player = await prisma.player.upsert({
            where: { Epic_Account_Id: epicAccountId },
            update: { username: username },
            create: { username: username, Epic_Account_Id: epicAccountId },
        });

        res.status(200).json({"message": "Save was successful"}, player);
    }
    catch (error) 
    {
        console.error("Login error:", error);
        res.status(500).json({ error: "Internal server error" });
    }
};