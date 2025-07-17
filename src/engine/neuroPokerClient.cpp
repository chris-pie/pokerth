#include "neuroPokerClient.h"
#include "configfile.h"


nlohmann::json chat_schema = nlohmann::json::parse(R"({
    "type": "object",
    "properties": {
        "message": {
            "type": "string"
        }
    }
})");




neuroPokerClient::neuroPokerClient(const std::string &uri, const std::string &game_name, std::ostream *output_stream,
    std::ostream *error_stream, int timeout) :
    NeuroGameClient(uri, game_name, output_stream, error_stream, timeout),
     chat(Action("chat", "Send a chat message", chat_schema)),
     notifier(NeuroNotifier::getInstance())
{
    init();
}

neuroPokerClient::neuroPokerClient(ConfigFile *config_file, const std::string &game_name) :
NeuroGameClient(config_file->readConfigString("NeuroUri"), game_name,  &std::cout, &std::cerr, config_file->readConfigInt("NeuroTimeout")),
chat(Action("chat", "Send a chat message", chat_schema)),
notifier(NeuroNotifier::getInstance()),
owned_output_stream(new std::ofstream(config_file->readConfigString("NeuroLogFile"), std::ofstream::app))
{
    output = owned_output_stream.get();
    error = owned_output_stream.get();
    init();
}

neuroPokerClient::~neuroPokerClient() {
    shutting_down = true;
    notifier.killListeners();
    condition.notify_all();
    if (game_thread.joinable()) {
        game_thread.join();
    }
    if (chat_thread.joinable()) {
        chat_thread.join();
    }
    if (log_thread.joinable()) {
        log_thread.join();
    }
    if (turn_thread.joinable()) {
        turn_thread.join();
    }
    if (owned_output_stream) owned_output_stream->close();
}

void neuroPokerClient::init() {
    sendStartup();
    game_thread = std::thread(&neuroPokerClient::awaitGameStart, this);
}

void neuroPokerClient::awaitGameStart() {
    while (!shutting_down) {
        player = notifier.awaitGameStart();
        if (shutting_down) {
            return;
        }
        chat_thread = std::thread(&neuroPokerClient::listenForChat, this);
        log_thread = std::thread(&neuroPokerClient::listenForLog, this);
        turn_thread = std::thread(&neuroPokerClient::listenForTurn, this);
        sendRegisterActions({chat});
        notifier.awaitGameEnd();
        if (chat_thread.joinable()) {
            chat_thread.join();
        }
        if (log_thread.joinable()) {
            log_thread.join();
        }
        if (turn_thread.joinable()) {
            turn_thread.join();
        }


    }
}

void neuroPokerClient::listenForChat() {
    while (!notifier.isKilled()) {
        std::string msg = notifier.getChat();
        if (msg != "")
            sendContext("Chat message: " + msg, false);
    }
}

void neuroPokerClient::listenForLog() {

    while (!notifier.isKilled()) {
        std::string msg = notifier.getLog();
        if (msg != "")
            sendContext(msg, notifier.getAndClearSilent());
    }
}

void neuroPokerClient::listenForTurn() {

    while (!notifier.isKilled()) {
        notifier.awaitTurn();
        if (!!notifier.isKilled()) {
            auto stateActions = player->getStateAndActions();
            forceDisposableActions(stateActions.first.dump(), "Please make your move. Do not tell anyone what your hole cards are!", true, stateActions.second);
        }
    }
}

void neuroPokerClient::handleMessage(NeuroResponse const &response) {
    //Only action that can be done at any moment.
    if (response.getName() == "chat") {
        try {
            nlohmann::json responseData = nlohmann::json::parse(response.getData());
            std::string message = responseData["message"];
            player->neuroSendChat(message);
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
            player->neuroFold();
        }
        else if (response.getName() == "call") {
            resp = "You called";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            player->neuroCheckCall();
        }
        else if (response.getName() == "check") {
            resp = "You checked";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            player->neuroCheckCall();
        }
        else if (response.getName() == "allin") {
            resp = "You went all in";
            sendUnregisterActions(disposableActions);
            sendActionResult(response, true, resp);
            waitingForForcedAction = false;
            player->neuroAllIn();
        }
        else if (response.getName() == "bet") {
            try {
                nlohmann::json responseData = nlohmann::json::parse(response.getData());
                int bet = responseData["bet"];
                if (player->neuroCanBet(bet) == false) {
                    sendActionResult(response, false, "Bet must be between YourCash and MinimumBetAmount");
                    return;
                }
                resp = "You bet";
                sendUnregisterActions(disposableActions);
                sendActionResult(response, true, resp);
                waitingForForcedAction = false;
                player->neuroBet(bet);
            } catch (nlohmann::json::exception &e) {
                sendActionResult(response, false, "Invalid action format.");
            }
        }
        else if (response.getName() == "raise") {
            try {
                nlohmann::json responseData = nlohmann::json::parse(response.getData());
                int bet = responseData["bet"];
                if (player->neuroCanBet(bet) == false) {
                    sendActionResult(response, false, "Raise must be between YourCash and MinimumRaiseAmount");
                    return;
                }
                resp = "You raised";
                sendUnregisterActions(disposableActions);
                sendActionResult(response, true, resp);
                waitingForForcedAction = false;
                player->neuroBet(bet);
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