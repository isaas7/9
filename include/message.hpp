#pragma once
#include <string>
#include <vector>
class Message {
public:
  Message(const std::string &msg) : message(msg){};
  std::string getMsgString() const { return message; }

private:
  std::string message;
};

class MessageService {
public:
  void addMessage(std::string &msg) { messages.emplace_back(msg); }
  std::vector<Message> getMessages() { return messages; }

private:
  std::vector<Message> messages;
};

class ChatService {
public:
  ChatService() { rooms_.emplace_back(); }
  void addRoom() { rooms_.emplace_back(); }
  std::vector<MessageService> &getMsgService() {
    return rooms_;
  } // Return reference

private:
  std::vector<MessageService> rooms_;
};
