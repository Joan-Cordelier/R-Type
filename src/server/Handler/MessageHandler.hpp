/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MessageHandler
*/

#ifndef MESSAGEHANDLER_HPP_
#define MESSAGEHANDLER_HPP_

#include "../../common/Data/MessageFactory.hpp"
#include "../Session/SessionManager.hpp"
#include <atomic>
#include <functional>

struct MoveData {
  uint32_t playerId;
  float vx;
  float vy;
};

struct ShootData {
  uint32_t playerId;
  float x;
  float y;
};

class MessageHandler {
public:
  using PlayerCallback = std::function<void(const Player &)>;
  using MoveCallback = std::function<void(const MoveData &)>;
  using ShootCallback = std::function<void(const ShootData &)>;
  using UpgradeCallback = std::function<void(uint32_t, uint8_t)>;

  MessageHandler(SessionManager &session, ThreadedQueue<DecodedMessage> &queue,
                 std::atomic<bool> &running);
  ~MessageHandler() = default;

  void processMessages();

  bool processSingleCycle();

  void setOnPlayerConnect(PlayerCallback callback) {
    _onPlayerConnect = callback;
  }
  void setOnPlayerDisconnect(PlayerCallback callback) {
    _onPlayerDisconnect = callback;
  }
  void setOnPlayerMove(MoveCallback callback) { _onPlayerMove = callback; }
  void setOnPlayerShoot(ShootCallback callback) { _onPlayerShoot = callback; }
  void setOnPlayerLink(std::function<void(uint32_t)> callback) {
    _onPlayerLink = callback;
  }
  void setOnUpgradeSelect(UpgradeCallback callback) {
    _onUpgradeSelect = callback;
  }

private:
  SessionManager &_session;
  ThreadedQueue<DecodedMessage> &_queue;
  std::atomic<bool> &_running;

  PlayerCallback _onPlayerConnect;
  PlayerCallback _onPlayerDisconnect;
  MoveCallback _onPlayerMove;
  ShootCallback _onPlayerShoot;
  std::function<void(uint32_t)> _onPlayerLink;
  UpgradeCallback _onUpgradeSelect;

  void dispatchMessage(DecodedMessage msg);

  void handleConnect(const DecodedMessage &msg);
  void handleLink(const DecodedMessage &msg);
  void handleDeath(const DecodedMessage &msg);
  void handleMoveInput(const DecodedMessage &msg);
  void handleShoot(const DecodedMessage &msg);
  void handleUpgradeSelect(const DecodedMessage &msg);
};

#endif /* !MESSAGEHANDLER_HPP_ */
