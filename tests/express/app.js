const express = require("express");
const bodyParser = require("body-parser");
const path = require("path");

const app = express();
const port = 3000;

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, "public")));
app.post("/api/messages", async (req, res) => {
  try {
    const { room } = req.body;

    const remoteResponse = await fetch("http://localhost:8080/api/messages", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ room }),
    });

    // Log raw output
    const rawOutput = await remoteResponse.text();
    console.log("Raw Output from Fetch:", rawOutput);

    if (!remoteResponse.ok) {
      const errorData = await remoteResponse.json();
      console.error("Error retrieving messages:", errorData.error);

      res
        .status(remoteResponse.status)
        .json({ success: false, message: errorData.error });
      return;
    }

    // Parse the outer 'messages' property
    const remoteData = JSON.parse(rawOutput);

    // Parse the inner JSON string
    const innerMessages = JSON.parse(remoteData.message || "[]");

    const response = {
      messages: innerMessages,
    };

    res.json(response);
  } catch (error) {
    console.error("Error retrieving messages:", error);

    res.status(500).json({ success: false, message: "Internal server error" });
  }
});
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
