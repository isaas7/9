#include "../../../../include/application/services/message/message.hpp"

Message::Message(const std::string &msg) : message_(msg) {}

std::string Message::getMessage() const { return message_; }

PrivateMessage::PrivateMessage(const std::string &msg, const User &sender,
                               const User &receiver)
    : Message(msg), sender_(sender), receiver_(receiver) {}

User PrivateMessage::getSender() const { return sender_; }

User PrivateMessage::getReceiver() const { return receiver_; }
