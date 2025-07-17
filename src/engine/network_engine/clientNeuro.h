#ifndef CLIENTNEURO_H
#define CLIENTNEURO_H

#include <neuroPokerClient.h>
#include "clientplayer.h"

class clientNeuro : public ClientPlayer, public NeuroPlayer  {
public:
    clientNeuro(
        ConfigFile *config_file,
        int id,
        unsigned uniqueId,
        const PlayerType &type,
        const std::string &name,
        const std::string &avatar,
        int sC,
        bool aS,
        bool sotS,
        int mB
    );
    ~clientNeuro() override;
    void neuroFold() override;
    std::pair<nlohmann::json, std::vector<NeuroWebsocketpp::Action>> getStateAndActions() override;
    void neuroCheckCall() override;
    void neuroAllIn() override;
    bool neuroCanBet(int bet) override;
    void neuroBet(int bet) override;
    void neuroSendChat(std::string message) override;

private:
    static std::string convertCardIntToString(int code);
    Action call;
    Action raise;
    Action fold;
    Action check;
    Action bet;
    Action allin;
};

#endif //CLIENTNEURO_H
