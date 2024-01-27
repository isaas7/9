#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

class Messages {
public:
  Messages(const std::string &message)
      : message(message), timestamp(std::chrono::system_clock::now()) {}
  const std::string &getMessage() const { return message; }
  const std::chrono::system_clock::time_point &getTimestamp() const {
    return timestamp;
  }

private:
  std::string message;
  std::chrono::system_clock::time_point timestamp;
};

class Room {
public:
  void addMessage(const std::string &message) {
    messages_.emplace_back(message);
  }
  const std::vector<Messages> &getMessages() const { return messages_; }

private:
  std::vector<Messages> messages_;
};

class ChatSystem {
public:
  void createRooms() {
    // Create two rooms
    rooms_.emplace_back(Room());
    rooms_.emplace_back(Room());
  }
  void sendMessageToRoom(int roomIndex, const std::string &message) {
    if (roomIndex >= 0 && roomIndex < rooms_.size()) {
      rooms_[roomIndex].addMessage(message);
    } else {
      std::cerr << "Invalid room index." << std::endl;
    }
  }
  const std::vector<Room> &getRooms() const { return rooms_; }

private:
  std::vector<Room> rooms_;
};
