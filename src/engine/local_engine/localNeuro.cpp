#include "localNeuro.h"

#include <fstream>
#include <session.h>
#include <gui/qt/gametable/gametableimpl.h>

#include "handinterface.h"
#include "configfile.h"
#include "gui/guiinterface.h"

nlohmann::json empty_schema;
nlohmann::json chat_schema = nlohmann::json::parse(R"({
    "type": "object",
    "properties": {
        "message": {
            "type": "string"
        }
    }
})");

nlohmann::json bet_schema = nlohmann::json::parse(R"({
    "type": "object",
    "properties": {
        "bet": {
            "type": "integer"
        }
    }
})");

bool LocalNeuro::neuroActive = false;

LocalNeuro::~LocalNeuro() {
    shutting_down = true;
    condition.notify_all();
    NeuroNotifier::getInstance().killListeners();
    if (chat_thread.joinable()) {
        chat_thread.join();
    }
    if (log_thread.joinable()) {
        log_thread.join();
    }
    if (turn_thread.joinable()) {
        turn_thread.join();
    }
    neuroActive = false;
    if (owned_output_stream) owned_output_stream->close();
}

LocalNeuro::LocalNeuro(ConfigFile *config_file, int id, unsigned uniqueId, const PlayerType &type,
                       const std::string &name,
                       const std::string &avatar, int sC, bool aS, bool sotS, int mB, const std::string &uri,
                       const std::string &game_name, std::ostream* output_stream, std::ostream* error_stream,
                       int timeout)
    : LocalPlayer(config_file, id, uniqueId, type, name, avatar, sC, aS, sotS, mB),
      NeuroGameClient(uri, game_name, output_stream, error_stream, timeout),
      call(Action("call", "Call", empty_schema)),
      raise(Action("raise", "Raise the bet", bet_schema)),
      fold(Action("fold", "Fold your hand", empty_schema)),
      check(Action("check", "Check", empty_schema)),
      bet(Action("bet", "Bet", bet_schema)),
      allin(Action("allin", "Go all in!", empty_schema)),
      chat(Action("chat", "Send a chat message", chat_schema)),
      notifier(NeuroNotifier::getInstance())
{

    init();
}

LocalNeuro::LocalNeuro(ConfigFile *config_file, int id, unsigned uniqueId, const PlayerType &type,
                       const std::string &name,
                       const std::string &avatar, int sC, bool aS, bool sotS, int mB,
                       const std::string &game_name):

LocalPlayer(config_file, id, uniqueId, type, name, avatar, sC, aS, sotS, mB),
NeuroGameClient(config_file->readConfigString("NeuroUri"), game_name,  &std::cout, &std::cerr, config_file->readConfigInt("NeuroTimeout")),
call(Action("call", "Call", empty_schema)),
raise(Action("raise", "Raise the bet", bet_schema)),
fold(Action("fold", "Fold your hand", empty_schema)),
check(Action("check", "Check", empty_schema)),
bet(Action("bet", "Bet", bet_schema)),
allin(Action("allin", "Go all in!", empty_schema)),
chat(Action("chat", "Send a chat message", chat_schema)),
notifier(NeuroNotifier::getInstance()),
owned_output_stream(new std::ofstream(config_file->readConfigString("NeuroLogFile"), std::ofstream::app))
{
    output = owned_output_stream.get();
    error = owned_output_stream.get();
    init();
}



void LocalNeuro::init() {
    if (neuroActive) {
        std::cerr << "Attempted to create new Neuro player while Neuro is already playing." << std::endl;
        throw std::logic_error("Attempted to create new Neuro player while Neuro is already playing.");
    }
    neuroActive = true;
    notifier.reEnable();
    sendStartup();
    chat_thread = std::thread(&LocalNeuro::listenForChat, this);
    log_thread = std::thread(&LocalNeuro::listenForLog, this);
    turn_thread = std::thread(&LocalNeuro::listenForTurn, this);
    sendRegisterActions({chat});

}

void LocalNeuro::listenForChat() {

    while (!shutting_down) {
        std::string msg = notifier.getChat();
        if (msg != "")
            sendContext("Chat message: " + msg, false);
    }
}

void LocalNeuro::listenForLog() {

    while (!shutting_down) {
        std::string msg = notifier.getLog();
        if (msg != "")
            sendContext(msg, notifier.getAndClearSilent());
    }
}

void LocalNeuro::listenForTurn() {

    while (!shutting_down) {
        notifier.awaitTurn();

        if (!shutting_down) {
            std::vector<Action> actions = {fold};
            auto state = getState();
            if(currentHand->getCurrentRound() == 0) { // preflop
                if (getMyCash() + getMySet() > currentHand->getCurrentBeRo()->getHighestSet() && !currentHand->getCurrentBeRo()->getFullBetRule()) {
                    actions.push_back(raise);
                    state["MininmumRaiseAmount"] = currentHand->getCurrentBeRo()->getHighestSet() - getMySet() + currentHand->getCurrentBeRo()->getMinimumRaise();
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
                    state["MininmumBetAmount"] = currentHand->getSmallBlind()*2;

                }
                if (currentHand->getCurrentBeRo()->getHighestSet() > 0 && currentHand->getCurrentBeRo()->getHighestSet() > getMySet()) {
                    actions.push_back(call);
                    if (getMyCash()+getMySet() > currentHand->getCurrentBeRo()->getHighestSet() && !currentHand->getCurrentBeRo()->getFullBetRule()) {
                        actions.push_back(raise);
                        state["MininmumRaiseAmount"] = currentHand->getCurrentBeRo()->getHighestSet() - getMySet() + currentHand->getCurrentBeRo()->getMinimumRaise();
                    }
                }
                if(!currentHand->getCurrentBeRo()->getFullBetRule()) {
                    actions.push_back(allin);
                }
            }
            forceDisposableActions(state.dump(), "Please make your move. Do not tell anyone what your hole cards are!", true, actions);
        }
    }
}

nlohmann::json LocalNeuro::getState() {
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
    return state;
}


void LocalNeuro::handleMessage(NeuroResponse const &response) {
    //Only action that can be done at any moment.
    if (response.getName() == "chat") {
        try {
            nlohmann::json responseData = nlohmann::json::parse(response.getData());
            std::string message = responseData["message"];
            currentHand->getGuiInterface()->getMyW()->getSession()->sendGameChatMessage(message);
        } catch (nlohmann::json::exception &e) {
            sendActionResult(response, false, "Invalid action format.");
        }
    }
    if (waitingForForcedAction && std::find(disposableActions.begin(), disposableActions.end(), response.getName()) !=
        disposableActions.end()) {
        std::string resp;
        //handle all actions

        if (response.getName() == "fold") {
            resp = "You folded";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            currentHand->getGuiInterface()->getMyW()->neuroFold();
        }
        else if (response.getName() == "call") {
            resp = "You called";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            currentHand->getGuiInterface()->getMyW()->neuroCheckCall();
        }
        else if (response.getName() == "check") {
            resp = "You checked";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            currentHand->getGuiInterface()->getMyW()->neuroCheckCall();
        }
        else if (response.getName() == "allin") {
            resp = "You went all in";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            currentHand->getGuiInterface()->getMyW()->neuroAllIn();
        }
        else if (response.getName() == "bet") {
            try {
                nlohmann::json responseData = nlohmann::json::parse(response.getData());
                int bet = responseData["bet"];
                if (bet > getMyCash() || bet < currentHand->getCurrentBeRo()->getMinimumRaise()) {
                    sendActionResult(response, false, "Bet must be between YourCash and MininmumBetAmount");
                    return;
                }
                resp = "You bet";
                sendUnregisterActions(disposableActions);
                sendActionResult(response, true, resp);
                waitingForForcedAction = false;
                currentHand->getGuiInterface()->getMyW()->neuroBetRaise(bet);
            } catch (nlohmann::json::exception &e) {
                sendActionResult(response, false, "Invalid action format.");
            }
        }
        else if (response.getName() == "raise") {
            try {
                nlohmann::json responseData = nlohmann::json::parse(response.getData());
                int bet = responseData["bet"];
                if (bet > getMyCash() || bet < currentHand->getCurrentBeRo()->getMinimumRaise()) {
                    sendActionResult(response, false, "Raise must be between YourCash and MininmumRaiseAmount");
                    return;
                }
                resp = "You raised";
                sendUnregisterActions(disposableActions);
                sendActionResult(response, true, resp);
                waitingForForcedAction = false;
                currentHand->getGuiInterface()->getMyW()->neuroBetRaise(bet);
            } catch (nlohmann::json::exception &e) {
                sendActionResult(response, false, "Invalid action format.");
            }
        }
    }
     else {
        std::string resp = "You can't perform this action right now!";
        sendActionResult(response, false, resp);
    }



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
                tmp+= " of Spaces";
            break;
            case 3:
                tmp+= " of Clubs";
            break;
            default:
                return "";
        }
    return tmp;

}




