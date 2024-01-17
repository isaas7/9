#include "../include/handler.hpp"

bool validate_token(
    const std::string &token,
    const std::unordered_map<std::string, SessionData> &session_storage) {
  auto session_it = session_storage.find(token);
  return session_it != session_storage.end() &&
         session_it->second.expiration_time > std::chrono::steady_clock::now();
}
size_t count_char(const std::string &target, char ch) {
  return std::count(target.begin(), target.end(), ch);
}

