#ifndef NEUROPOKERCLIENT_H
#define NEUROPOKERCLIENT_H
#include <third_party/neuro-sdk-websocketpp/NeuroGameSdkWebsocketpp.hpp>
#include <NeuroNotifier.h>
#include <fstream>
#include <NeuroPlayer.h>
#include <configfile.h>


using NeuroWebsocketpp::NeuroResponse;
using NeuroWebsocketpp::Action;


class neuroPokerClient: public NeuroWebsocketpp::NeuroGameClient {
public:
    neuroPokerClient(const std::string &uri, const std::string &game_name, std::ostream *output_stream,
        std::ostream *error_stream, int timeout);

    neuroPokerClient(ConfigFile *config_file, const std::string &game_name);
    ~neuroPokerClient() override;


protected:
    void handleMessage(const NeuroWebsocketpp::NeuroResponse &response) override;


private:
    void listenForChat();
    void listenForLog();
    void listenForTurn();
    void init();

    void awaitGameStart();

    Action chat;
    std::thread chat_thread;
    std::thread log_thread;
    std::thread turn_thread;
    std::thread game_thread;
    NeuroNotifier& notifier;
    std::unique_ptr<std::ofstream> owned_output_stream;
    NeuroPlayer* player = nullptr;
};

#endif //NEUROPOKERCLIENT_H
