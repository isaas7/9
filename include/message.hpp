#pragma once
#include <string>
#include <vector>
class Message {
public:
  Message(std::string &msg) : message(msg){};

private:
  std::string message;
};

class MessageService {
public:
  void addMessage(std::string msg) { messages.emplace_back(msg); }
  std::vector<Message> getMessages() { return messages; }

private:
  std::vector<Message> messages;
};
