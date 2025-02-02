#include "NeuroNotifier.h"
NeuroNotifier& NeuroNotifier::getInstance()
{
    static std::unique_ptr<NeuroNotifier> instance(new NeuroNotifier());
    return *instance;
}

NeuroNotifier::NeuroNotifier()
{

}

void NeuroNotifier::reEnable(){
  killed = false;
  }

void NeuroNotifier::sendChatMessage(const std::string& message)
{
        std::lock_guard<std::mutex> lock(mutexChat);
        chatStream << message << std::endl;
        newChat = true;
        conditionChat.notify_one();
}

void NeuroNotifier::sendLogMessage(const std::string& message, bool silent)
{
        std::lock_guard<std::mutex> lock(mutexLog);
        if (!silent)
          this->silent = false;
        logStream << message << std::endl;
        newLog = true;
        conditionLog.notify_one();
}

void NeuroNotifier::notifyTurnStart()
{

        std::lock_guard<std::mutex> lock(mutexTurn);
        yourTurn = true;
        conditionTurn.notify_one();
}

std::string NeuroNotifier::getChat()
{
  std::unique_lock<std::mutex> lock(mutexChat);
  conditionChat.wait(lock, [this]{return newChat || killed;});
  if(killed) return "";
  std::string messages = chatStream.str();
  chatStream.str("");
  chatStream.clear();
  newChat = false;
  return messages;
}

std::string NeuroNotifier::getLog() {
  std::unique_lock<std::mutex> lock(mutexLog);
  conditionLog.wait(lock, [this]{return newLog || killed;});
  if(killed) return "";
  std::string messages = logStream.str();
  logStream.str("");
  logStream.clear();
  newLog = false;
  return messages;
}

void NeuroNotifier::awaitTurn()
{
 std::unique_lock<std::mutex> lock(mutexTurn);
 conditionTurn.wait(lock, [this]{return yourTurn || killed;});
 yourTurn = false;
}

void NeuroNotifier::killListeners()

{
  killed = true;
  std::unique_lock<std::mutex> lock1(mutexTurn);
  yourTurn = false;
  std::unique_lock<std::mutex> lock2(mutexLog);
  logStream.str("");
  logStream.clear();
  std::unique_lock<std::mutex> lock3(mutexChat);
  newLog = false;
  chatStream.str("");
  chatStream.clear();
  newChat = false;
  conditionTurn.notify_one();
  conditionChat.notify_one();
  conditionLog.notify_one();
}

bool NeuroNotifier::getAndClearSilent()
    {
  bool ret = silent;
  silent = true;
  return ret;
  }
