const express = require('express');
const http = require('http');
const path = require('path');
const app = express();
const bodyParser = require('body-parser');
const messages = []
app.use(express.static(path.join(__dirname, 'public')));
app.use(bodyParser.json());
app.get('/api/token', async (req, res) => {
  try {
    // Make a GET request to localhost:8080/api/token
    const tokenResponse = await fetch('http://localhost:8080/api/token');

    if (tokenResponse.ok) {
      const token = await tokenResponse.text();
      res.send(token);
    } else {
      console.error('Failed to get token:', tokenResponse.statusText);
      res.status(500).send('Internal Server Error');
    }
  } catch (error) {
    console.error('Error during token retrieval:', error.message);
    res.status(500).send('Internal Server Error');
  }
});
app.post('/api/messages', async (req, res) => {
  try {
    // Extract the token from the request body
    const tokenFromBody = req.body.token;

    // Make a GET request to localhost:8080/api/heartbeat with the token in the body
    const messagesResponse = await fetch('http://localhost:8080/api/messages', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ token: tokenFromBody }),
    });

    if (messagesResponse.ok) {
      const messagesText = await messagesResponse.text();
      res.send(messagesText);     
    } else {
      console.error(messagesResponse.statusText);
      res.status(500).send('Internal Server Error');
    }
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});
app.post('/api/messages/send', async (req, res) => {
  try {
    // Extract the token and message from the request body
    const { token, message } = req.body;

    // Validate the token (You may want to implement a more secure token validation)
    // For demonstration purposes, I'm assuming a simple validation here
    if (token && token.startsWith('Token_')) {
      // Make a POST request to localhost:8080/api/messages/send
      const sendMessageResponse = await fetch('http://localhost:8080/api/messages/send', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ token, message }),
      });

      if (sendMessageResponse.ok) {
        console.log(await sendMessageResponse.text());
        res.status(200).send('Message sent successfully.');
      } else {
        console.error(sendMessageResponse.statusText);
        res.status(500).send('Internal Server Error');
      }
    } else {
      // Respond with an error if the token is invalid
      res.status(400).send('Invalid token.');
    }
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});app.post('/api/heartbeat', async (req, res) => {
  try {
    // Extract the token from the request body
    const tokenFromBody = req.body.token;

    // Make a GET request to localhost:8080/api/heartbeat with the token in the body
    const heartbeatResponse = await fetch('http://localhost:8080/api/heartbeat', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ token: tokenFromBody }),
    });

    if (heartbeatResponse.ok) {
      console.log(await heartbeatResponse.text());
      res.send('Heartbeat received successfully.');     
    } else {
      console.error(heartbeatResponse.statusText);
      res.status(500).send('Internal Server Error');
    }
  } catch (error) {
    console.error(error.message);
    res.status(500).send('Internal Server Error');
  }
});

const PORT = 9090;
const server = http.createServer(app);
server.listen(PORT, () => {
  console.log(`Express app listening on port ${PORT}`);
});




