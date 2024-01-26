const express = require('express');
const bodyParser = require('body-parser');
const path = require('path'); // Import the path module
const app = express();
const port = 3000;

app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));
const messages = [];

app.post('/api/messages/send', async (req, res) => {
  try {
    const { message } = req.body;
    messages.push({ message });
    //console.log(`Message received: ${message}`);

    await fetch('http://localhost:8080/api/messages/send', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ message }),
    });

    res.json({ success: true, message: 'Message sent successfully' });
  } catch (error) {
    console.error('Error processing message:', error);
    res.status(500).json({ success: false, message: 'Internal server error' });
  }
});

app.get('/api/messages', async (req, res) => {
  try {
    const remoteResponse = await fetch('http://localhost:8080/api/messages');
    const remoteData = await remoteResponse.json();

    const response = {
      messages: remoteData.messages.map((msg) => ({
        message: msg.message,
        timestamp: msg.timestamp,
      })),
    };
    //console.log('Messages retrieved:', response);
    res.json(response);
  } catch (error) {
    console.error('Error retrieving messages:', error);
    res.status(500).json({ success: false, message: 'Internal server error' });
  }
});

app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
