const express = require("express");
const bodyParser = require("body-parser");
const path = require("path");
const axios = require("axios");

const app = express();
const port = 3000;

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, "public")));

app.post("/api/messages", async (req, res) => {
  try {
    const { room } = req.body;

    // Make a request to localhost:8080/api/messages
    const apiResponse = await axios.post("http://localhost:8080/api/messages", {
      room,
    });

    // Extract messages from the API response
    const messages = apiResponse.data.messages || [];

    // Send the messages as the response
    return res.json({ messages });
  } catch (error) {
    console.error("Error:", error.message);
    return res.status(500).json({ error: "Internal server error" });
  }
});

app.post("/api/messages/send", async (req, res) => {
  try {
    const { room, message } = req.body;

    // Make a request to http://localhost:8080/api/messages/send
    const apiResponse = await axios.post(
      "http://localhost:8080/api/messages/send",
      {
        room,
        message,
      },
    );

    // Check if the API response indicates success
    if (apiResponse.data.message === "success") {
      return res.json({ message: "success" });
    } else {
      return res.status(500).json({ error: "Failed to send message" });
    }
  } catch (error) {
    console.error("Error:", error.message);
    return res.status(500).json({ error: "Internal server error" });
  }
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
