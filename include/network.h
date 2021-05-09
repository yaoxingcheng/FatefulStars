#ifndef __NETWORK_H__
#define __NETWORK_H__

class NetworkManager {
public:
    NetworkManager(Game* game);
    ~NetworkManager();

    void StartServer();
    void StartClient();
    
private:
    Game* game;

};

#endif
