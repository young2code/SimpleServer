#pragma once

#include <string>
#include <vector>

#include "TSingleton.h"
#include "ListenSocket.h"
#include "ClientSocket.h"

#include <rapidjson/document.h>

class Server : public TSingleton<Server>
{
public:
	Server();
	~Server();

public:
	bool Init(UINT port = 0);
	void Shutdown();

	void OnAccept(int nErrorCode);
	void OnReceive(ClientSocket* socket, const rapidjson::Document& jsonData);

private:
	ListenSocket mListenSocket;
	std::vector<ClientSocket*> mClientSockets;
};