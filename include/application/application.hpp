#pragma once
#include "../util/util.hpp"
#include "services/message/service.hpp"
#include "services/user/service.hpp"
#include <memory>

class Application {
public:
  Application();
  virtual ~Application();

  std::shared_ptr<MessageService> getMessageService() const;
  std::shared_ptr<PrivateMessageService> getPrivateMessageService() const;
  std::shared_ptr<UserService> getUserService() const;

private:
  std::shared_ptr<MessageService> messageService_;
  std::shared_ptr<PrivateMessageService> privateMessageService_;
  std::shared_ptr<UserService> userService_;
};
