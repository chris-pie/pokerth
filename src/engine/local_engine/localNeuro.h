#ifndef LOCALNEURO_H
#define LOCALNEURO_H

#include <neuroPokerClient.h>
#include "localplayer.h"

class LocalNeuro : public LocalPlayer, public NeuroPlayer {
  public:
    LocalNeuro(
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
    ~LocalNeuro() override;
    void neuroFold() override;
    std::pair<nlohmann::json, std::vector<NeuroWebsocketpp::Action>> getStateAndActions() override;
    void neuroCheckCall() override;
    void neuroAllIn() override;
    bool neuroCanBet(int bet) override;
    void neuroBet(int bet) override;
    void neuroSendChat(std::string message) override;
    int getMinRaise();

private:
    static std::string convertCardIntToString(int code);
    Action call;
    Action raise;
    Action fold;
    Action check;
    Action bet;
    Action allin;
};

#endif //LOCALNEURO_H
