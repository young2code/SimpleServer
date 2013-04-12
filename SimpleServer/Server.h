#pragma once

#include <string>
#include <vector>

#include "TSingleton.h"
#include "ListenSocket.h"

#include <rapidjson/document.h>

class ClientSocket;

class Server : public TSingleton<Server>
{
public:
	Server();
	~Server();

public:
	bool Init(UINT port = 0);
	void Shutdown();

	void OnAccept(int nErrorCode);
	void OnClose(ClientSocket* socket);
	void OnReceive(ClientSocket* socket, const rapidjson::Document& jsonData);

private:
	ListenSocket mListenSocket;
	std::vector<ClientSocket*> mClientSockets;
};