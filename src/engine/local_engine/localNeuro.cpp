#include "localNeuro.h"
#include <gui/qt/gametable/gametableimpl.h>
#include "handinterface.h"
#include "configfile.h"
#include "session.h"

LocalNeuro::LocalNeuro(ConfigFile *config_file, int id, unsigned uniqueId, const PlayerType &type,
                       const std::string &name,
                       const std::string &avatar, int sC, bool aS, bool sotS, int mB)
    : LocalPlayer(config_file, id, uniqueId, type, name, avatar, sC, aS, sotS, mB),
call(Action("call", "Call", empty_schema)),
raise(Action("raise", "Raise the bet", bet_schema)),
fold(Action("fold", "Fold your hand", empty_schema)),
check(Action("check", "Check", empty_schema)),
bet(Action("bet", "Bet", bet_schema)),
allin(Action("allin", "Go all in!", empty_schema))
{

    NeuroNotifier::getInstance().reEnable(this);
}

LocalNeuro::~LocalNeuro() {
    NeuroNotifier::getInstance().killListeners();
}

std::pair<nlohmann::json, std::vector<Action>> LocalNeuro::getStateAndActions() {
    nlohmann::json state;
    int cards[2];
    getMyCards(cards);
    int communityCards[5];
    currentHand->getBoard()->getMyCards(communityCards);
    switch (currentHand->getCurrentRound()) {
        case GAME_STATE_PREFLOP:
            state["Round"] = "PRE-FLOP";
            break;
        case GAME_STATE_FLOP:
            state["Round"] = "FLOP";
        break;
        case GAME_STATE_TURN:
            state["Round"] = "TURN";
        break;
        case GAME_STATE_RIVER:
            state["Round"] = "RIVER";
        break;
        case GAME_STATE_POST_RIVER:
            state["Round"] = "POST-RIVER";
        break;
        default:
            state["Round"] = "UNKNOWN";
    }
    state["HoleCards"] = {convertCardIntToString(cards[0]), convertCardIntToString(cards[1])};
    state["CommunityCards"] = nlohmann::json::array();
    if (currentHand->getCurrentRound() >= GAME_STATE_FLOP) {
        state["CommunityCards"].push_back(convertCardIntToString(communityCards[0]));
        state["CommunityCards"].push_back(convertCardIntToString(communityCards[1]));
        state["CommunityCards"].push_back(convertCardIntToString(communityCards[2]));
    }
    if (currentHand->getCurrentRound() >= GAME_STATE_TURN) {
        state["CommunityCards"].push_back(convertCardIntToString(communityCards[3]));

    }
    if (currentHand->getCurrentRound() >= GAME_STATE_RIVER) {
        state["CommunityCards"].push_back(convertCardIntToString(communityCards[4]));
    }
    state["Pot"] = currentHand->getBoard()->getPot();
    state["YourSet"] = getMySet();
    state["YourCash"] = getMyCash();
    state["CurrentHighestSet"] = currentHand->getCurrentBeRo()->getHighestSet();
    nlohmann::json players = nlohmann::json::array();
    auto playerList = currentHand->getSeatsList();
    for (auto & it : *playerList) {
        nlohmann::json playerInfo = nlohmann::json::object();
        playerInfo["Name"] = it->getMyName();
        playerInfo["Cash"] = it->getMyCash();
        playerInfo["Set"] = it->getMySet();
        switch (it.get()->getMyAction()) {
            case PLAYER_ACTION_FOLD:
                playerInfo["Status"] = "Folded";
            break;
            case PLAYER_ACTION_ALLIN:
                playerInfo["Status"] = "All in";
            break;
            default:
                if (it->getMyActiveStatus()){
                    playerInfo["Status"] = "Playing";
                }
                else {
                    playerInfo["Status"] = "Eliminated";
            }
        }
    }

    std::vector<Action> actions = {fold};

    if(currentHand->getCurrentRound() == 0) { // preflop
        if (getMyCash() + getMySet() > currentHand->getCurrentBeRo()->getHighestSet() && !currentHand->getCurrentBeRo()->getFullBetRule()) {
            actions.push_back(raise);
            state["MinimumRaiseAmount"] = getMinRaise();
        }

        if (getMySet() == currentHand->getCurrentBeRo()->getHighestSet() &&  getMyButton() == 3) {
            actions.push_back(check);
        } else {
            actions.push_back(call);
        }

        if(!currentHand->getCurrentBeRo()->getFullBetRule()) {
            actions.push_back(allin);
        }
    } else { // flop,turn,river

        if (currentHand->getCurrentBeRo()->getHighestSet() == 0) {

            actions.push_back(check);
            actions.push_back(bet);
            state["MinimumBetAmount"] = currentHand->getSmallBlind()*2;

        }
        if (currentHand->getCurrentBeRo()->getHighestSet() > 0 && currentHand->getCurrentBeRo()->getHighestSet() > getMySet()) {
            actions.push_back(call);
            if (getMyCash()+getMySet() > currentHand->getCurrentBeRo()->getHighestSet() && !currentHand->getCurrentBeRo()->getFullBetRule()) {
                actions.push_back(raise);
                state["MinimumRaiseAmount"] = getMinRaise();
            }
        }
        if(!currentHand->getCurrentBeRo()->getFullBetRule()) {
            actions.push_back(allin);
        }
    }

    return std::make_pair(state, actions);


}

void LocalNeuro::neuroFold() {
    currentHand->getGuiInterface()->getMyW()->neuroFold();
}

void LocalNeuro::neuroCheckCall() {
    currentHand->getGuiInterface()->getMyW()->neuroCheckCall();
}

void LocalNeuro::neuroAllIn() {
    currentHand->getGuiInterface()->getMyW()->neuroAllIn();
}

bool LocalNeuro::neuroCanBet(int bet) {
    if (bet > getMyCash() || bet < getMinRaise()) {
        return false;
    }
    return true;
}

void LocalNeuro::neuroBet(int bet) {
    currentHand->getGuiInterface()->getMyW()->neuroBetRaise(bet);
}

void LocalNeuro::neuroSendChat(std::string message) {
    currentHand->getGuiInterface()->getMyW()->getSession()->sendGameChatMessage(message);

}

int LocalNeuro::getMinRaise() {
    return currentHand->getCurrentBeRo()->getHighestSet() - getMySet() + currentHand->getCurrentBeRo()->getMinimumRaise();
}


std::string LocalNeuro::convertCardIntToString(int code) {
    std::string tmp;
    if (code == -1)
        return "";
    switch(code%13) {
        case 0:
            tmp = "2";
        break;
        case 1:
            tmp = "3";
        break;
        case 2:
            tmp = "4";
        break;
        case 3:
            tmp = "5";
        break;
        case 4:
            tmp = "6";
        break;
        case 5:
            tmp = "7";
        break;
        case 6:
            tmp = "8";
        break;
        case 7:
            tmp = "9";
        break;
        case 8:
            tmp = "10";
        break;
        case 9:
            tmp = "Jack";
        break;
        case 10:
            tmp = "Queen";
        break;
        case 11:
            tmp = "King";
        break;
        case 12:
            tmp = "Ace";
        break;
        default:
            return "";
    }

        switch(code/13) {
            case 0:
                tmp+= " of Diamonds";
            break;
            case 1:
                tmp+= " of Hearts";
            break;
            case 2:
                tmp+= " of Spades";
            break;
            case 3:
                tmp+= " of Clubs";
            break;
            default:
                return "";
        }
    return tmp;

}




