#ifndef NEURONOTIFIER_H
#define NEURONOTIFIER_H
#include <sstream>
#include <string>
#include <mutex>
#include <condition_variable>

#include "NeuroPlayer.h"


class NeuroNotifier {
public:
    static NeuroNotifier& getInstance();
    void sendChatMessage(const std::string& message);
    void sendLogMessage(const std::string& message, bool silent = true);
    void notifyTurnStart();
    std::string getChat();
    std::string getLog();
    void awaitTurn();
    bool getAndClearSilent();
    bool isKilled();
    NeuroNotifier(const NeuroNotifier&) = delete;
    NeuroNotifier& operator=(const NeuroNotifier&) = delete;

    void killListeners();
    void reEnable(NeuroPlayer* player);
    NeuroPlayer* awaitGameStart();

    void awaitGameEnd();

private:
    NeuroNotifier();
    std::ostringstream logStream;
    std::ostringstream chatStream;
    std::mutex mutexChat;
    std::mutex mutexLog;
    std::mutex mutexTurn;
    std::mutex mutexGameStart;
    std::mutex mutexGameEnd;
    std::condition_variable conditionLog;
    std::condition_variable conditionChat;
    std::condition_variable conditionTurn;
    std::condition_variable conditionGameStart;
    std::condition_variable conditionGameEnd;
    bool newChat = false;
    bool newLog = false;
    bool yourTurn = false;
    bool killed = false;
    bool silent = true;
    NeuroPlayer* player = nullptr;
};


#endif //NEURONOTIFIER_H
