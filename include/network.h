#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <string>
#include <thread>

class Button;
class TextInput;

enum NetworkRole {
    SERVER,
    CLIENT,
};

enum NetworkStatus {
    OFFLINE,
    WAITING,
    CONNECTED,
};

class NetworkManager {
public:
    static const int PORT = 24678;
    static const int BUFFER_SIZE = 1 << 23;

    NetworkManager(Game* game);
    ~NetworkManager();

    void Init();
    void Update();

    void DrawMainButton();
    void DrawSettingsPanel();
    
    void StartServer(int port);
    void StartClient(std::string host, int port);
    
private:
    Game* game;
    Button* mainBtn;
    Button *mainBtn2;
    Button* closeBtn;
    Button *actionBtn;
    TextInput *textInput;
    
    NetworkRole role;
    NetworkStatus status;

    char* buffer;
    int buffer_cur;

    std::thread* listenThread;
    std::thread* connectThread;
    int server_sockfd = -1;
    int client_sockfd = -1;

    bool parseURL(std::string url, std::string& host, int& port);
    void stopConnections();
    void startCoummunication(int fd);
};

#endif
