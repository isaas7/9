#pragma once
#include "../../../util/util.hpp"
#include "../user/user.hpp"
#include "game.hpp"
class GameService : public Service {
public:
  GameService();
  User playGame(const User &p1, const User &p2);

private:
  Game game_;
};
