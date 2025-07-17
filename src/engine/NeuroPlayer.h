#ifndef NEUROPLAYER_H
#define NEUROPLAYER_H

#include <string>
#include "neuro-game-sdk/Websocketpp/json.hpp"
#include "neuro-game-sdk/Websocketpp/NeuroGameSdkWebsocketpp.hpp"




class NeuroPlayer {
public:
    virtual ~NeuroPlayer() = default;
    virtual std::pair<nlohmann::json, std::vector<NeuroWebsocketpp::Action>> getStateAndActions() = 0;
    virtual void neuroFold() = 0;
    virtual void neuroCheckCall() = 0;
    virtual void neuroAllIn() = 0;

    virtual bool neuroCanBet(int bet) = 0;
    virtual void neuroBet(int bet) = 0;
    virtual void neuroSendChat(std::string message) = 0;

protected:

    nlohmann::json empty_schema;
    nlohmann::json bet_schema = nlohmann::json::parse(R"({
    "type": "object",
    "properties": {
        "bet": {
            "type": "integer"
        }
    }
})");

};
#endif //NEUROPLAYER_H
