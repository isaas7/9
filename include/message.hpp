#pragma once
#include <string>
#include <vector>
class Message {
public:
  Message(const std::string &msg) : message_(msg){};
  std::string getMsgString() const { return message_; }

private:
  std::string message_;
  // timestamps
};

class MessageService {
public:
  void addMessage(std::string &msg) { messages.emplace_back(msg); }
  std::vector<Message> getMessages() { return messages; }

private:
  std::vector<Message> messages;
};

class User {
public:
  User(int id, const std::string &username, const std::string &password)
      : id_(id), username_(username), password_(password){};
  int getId() { return id_; }
  std::string getUsername() const { return username_; }
  std::string getPassword() const { return password_; }

private:
  int id_;
  std::string username_;
  std::string password_;
};
class UserService {
public:
  void addUser(std::string &username, std::string &password) {
    if (users_.size() == 0)
      users_.emplace_back(1, username, password);
    else
      users_.emplace_back(users_.size() + 1, username, password);
  }
  std::vector<User> &getUsers() { return users_; }
  UserService(){};

private:
  std::vector<User> users_;
};

class ChatService {
public:
  ChatService() { rooms_.emplace_back(); }
  void addRoom() { rooms_.emplace_back(); }
  std::vector<MessageService> &getRooms() { return rooms_; }

private:
  std::vector<MessageService> rooms_;
};
