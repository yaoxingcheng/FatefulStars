#include "game.h"
#include "common.h"
#include "network.h"
#include <unistd.h>
#include <cctype>

NetworkManager::NetworkManager(Game* game) : game(game) {
    mainBtn = nullptr;
    mainBtn2 = nullptr;
    closeBtn = nullptr;
    actionBtn = nullptr;
}

NetworkManager::~NetworkManager() {
    delete mainBtn;
    delete mainBtn2;
    delete closeBtn;
    delete actionBtn;
}

void NetworkManager::Init() {
    int padding = 20;
    Vector2 btnPos((Vector2){(float)game->screenWidth - padding - 60, 20});
    mainBtn = new Button(game, "resources/btn/network1.png");
    mainBtn->SetPosition(btnPos);
    mainBtn2 = new Button(game, "resources/btn/network2.png");
    mainBtn2->SetPosition(btnPos);

    closeBtn = new Button(game, "resources/btn/close.png");
    closeBtn->SetPosition(btnPos);

    actionBtn = new Button(game, "resources/btn/action.png");

    int W = game->screenWidth;
    int H = game->screenHeight;
    textInput = new TextInput(game, (Rectangle){0.24f * W, 0.33f * H, 0.55f * W, 0.08f * H});
    textInput->SetPlaceholder("Enter [ip]:[port]");

    actionBtn->SetPosition((Vector2){0.29f * W, 0.54f * H});
}

void NetworkManager::Update() {
    if (game->GetScene() == MAIN) {
        if (status == CONNECTED) {
            if (mainBtn2->Clicked()) {
                status = OFFLINE;
            }
        } else {
            if (mainBtn->Clicked()) {
                game->SetScene(NETWORK_SETTINGS);       
            }
        }
    } else if (game->GetScene() == NETWORK_SETTINGS) {
        string serverUrl = textInput->TextReadyChange();
        textInput->SetText(serverUrl);

        if (closeBtn->Clicked()) {
            game->SetScene(MAIN);
        }

        if (actionBtn->Clicked()) {
            if (status == OFFLINE) {
                string url = textInput->GetText();
                string host;
                int port;
                if (url == "" || parseURL(url, host, port)) {
                    status = WAITING;
                    textInput->SetReadOnly(true);
                    if (url == "") {
                        role = SERVER;
                        StartServer(PORT);
                    } else {
                        role = CLIENT;
                        StartClient(host, port);
                    }
                }
            } else {
                status = OFFLINE;
                textInput->SetReadOnly(false);
            }
        }
    }
}

void NetworkManager::DrawSettingsPanel() {
    closeBtn->Draw();
    actionBtn->Draw();
    textInput->Draw();

    int W = game->screenWidth;
    int H = game->screenHeight;
    DrawText("Play with your friends!", int(0.22 * W), int(0.23 * H), int(0.05 * H), WHITE);

    int padding = 0;
    string actionText = "Start server";

    if (textInput->GetText() != "") {
        padding = int(0.02 * W);
        actionText = "Dial server";
    }

    if (status == WAITING) {
        padding = int(0.08 * W);
        actionText = "Cancel";
    }

    DrawText(actionText.c_str(), int(0.38 * W) + padding, int(0.56 * H), 32, BLACK);

    string msgText = "";

    if (status == WAITING) {
        if (role == SERVER) {
            padding = int(0.1 * W);
            msgText = "Listening on port " + std::to_string(PORT) + " ...";
        } else {
            padding = int(0.03 * W);
            msgText = "Connecting to " + textInput->GetText() + " ...";
        }
    }
    if (msgText != "") {
        DrawText(msgText.c_str(), int(0.23 * W) + padding, int(0.7 * H), 24, WHITE);
    }
}

void NetworkManager::DrawMainButton() {
    if (status == CONNECTED) {
        mainBtn2->Draw();
    } else {
        mainBtn->Draw();
    }
}

void NetworkManager::stopConnections() {
    status = OFFLINE;
    textInput->SetReadOnly(false);
}

bool NetworkManager::parseURL(string url, string& host, int& port) {
    int n = url.length();
    int i = 0;
    int dotCount = 0;
    while (i < n) {
        if (url[i] != '.' && url[i] != ':' && !isdigit(url[i])) {
            return false;
        }
        if (url[i] == '.') dotCount ++;
        if (url[i] == ':') break;
        i ++;
    }
    if (dotCount != 3) return false;
    int p = i ++;
    host = url.substr(0, p);
    while (i < n) {
        if (!isdigit(url[i])) {
            return false;
        }
        i ++;
    }
    if (p < n) {
        port = stoi(url.substr(p + 1, n - p - 1));
    } else {
        port = 0;
    }
    return true;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

void NetworkManager::StartServer(int port) {
    using std::thread;
    if (server_sockfd >= 0) return;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return;
    server_sockfd = sockfd;
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(sockfd, (const struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        close(sockfd);
        stopConnections();
        return;
    }
    if (listen(sockfd, 20) < 0) {
        close(sockfd);
        stopConnections();
        return;
    }
    listenThread = new thread([this, sockfd]() {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int conn = accept(sockfd, (struct sockaddr*)&client_addr, &len);
        if (conn < 0) {
            stopConnections();
            return;
        }
        if (role != SERVER || status != WAITING) {
            close(conn);
            close(sockfd);
        }
        printf("Accept connection: %d\n", conn);
        startCoummunication(conn);
    });

    listenThread->detach();
}

void NetworkManager::StartClient(string host, int port) {
    using std::thread;
    if (!port) {
        port = PORT;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return;
    client_sockfd = sockfd;

    connectThread = new thread([this, sockfd, host, port]() {
        struct sockaddr_in remote_addr;
        memset(&remote_addr, 0, sizeof(remote_addr));
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr(host.c_str());
        remote_addr.sin_port = htons(port);
        int conn = connect(sockfd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr));
        if (conn < 0) {
            stopConnections();
            close(sockfd);
            return;
        }
        if (role != CLIENT || status != WAITING) {
            close(sockfd);
        }
        startCoummunication(sockfd);
    });

    connectThread->detach();
}

#include "game_state.h"

void NetworkManager::startCoummunication(int sockfd) {
    status = CONNECTED;
    game->ResetGame();
    game->SetScene(MAIN);
    GameState* state = new GameState();
    int len = sizeof(GameState);

    auto close_all = [&]() {
        if (server_sockfd >= 0) {
            close(server_sockfd);
            server_sockfd = -1;
        }
        if (client_sockfd >= 0) {
            close(client_sockfd);
            client_sockfd = -1;
        }
        close(sockfd);
        status = OFFLINE;
    };

    while (true) {
        game->DumpState(state);
        send(sockfd, state, len, 0);
        if (state->finished) {
            break;
        }
        if (recv(sockfd, state, len, MSG_WAITALL) == len) {
            game->LoadState(state);
            if (state->finished) {
                break;
            }
        }
        usleep(10 * 1000);
    }
    close_all();
}

bool NetworkManager::IsConnected() {
    return status == CONNECTED;
}
