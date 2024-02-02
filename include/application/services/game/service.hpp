#pragma once
#include "../../../util/util.hpp"
#include "../user/user.hpp"
#include "game.hpp"
class GameService : public Service {
public:
  GameService();

private:
  Game game_;
};
