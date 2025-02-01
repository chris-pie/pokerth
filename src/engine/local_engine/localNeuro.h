#ifndef LOCALNEURO_H
#define LOCALNEURO_H
#include "localPlayer.h"
#include <third_party/NeuroIntegration/neuro-game-sdk/Websocketpp/NeuroGameSdkWebsocketpp.hpp>

#include <NeuroNotifier.h>

#include <string>
#include <bits/std_thread.h>
using NeuroWebsocketpp::NeuroResponse;
using NeuroWebsocketpp::Action;



class LocalNeuro : public LocalPlayer, public NeuroWebsocketpp::NeuroGameClient {
  public:

    LocalNeuro(ConfigFile *config_file, int id, unsigned uniqueId, const PlayerType &type, const std::string &name,
      const std::string &avatar, int sC, bool aS, bool sotS, int mB, const std::string &uri,
      const std::string &game_name, std::ostream* output_stream, std::ostream* error_stream, int timeout = -1);

    LocalNeuro(ConfigFile *config_file, int id, unsigned uniqueId, const PlayerType &type, const std::string &name,
      const std::string &avatar, int sC, bool aS, bool sotS, int mB,
      const std::string &game_name);



    void handleMessage(NeuroResponse const& response) override;


    ~LocalNeuro() override;

private:
    static std::string convertCardIntToString(int code);

    void listenForChat();
    void listenForLog();
    void listenForTurn();

    void init();

    nlohmann::json getState();

    Action call;
    Action raise;
    Action fold;
    Action check;
    Action bet;
    Action allin;
    Action chat;
    std::thread chat_thread;
    std::thread log_thread;
    std::thread turn_thread;
    NeuroNotifier& notifier;
    static bool neuroActive;
    std::unique_ptr<std::ofstream> owned_output_stream;
    bool shutting_down = false;

};



#endif //LOCALNEURO_H
