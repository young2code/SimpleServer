#include "stdafx.h"

#include "Server.h"
#include "ClientSocket.h"
#include "LogMan.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

Server::Server()
{
}


Server::~Server()
{
}


bool Server::Init(UINT port)
{
	if (!mListenSocket.Create(port))
	{
		LOG("Server::Init() - faied to create a listener socket.");
		return false;
	}

	BOOL reuse = TRUE;
	mListenSocket.SetSockOpt(SO_REUSEADDR, &reuse, sizeof(BOOL));

	if (!mListenSocket.Listen())
	{
		LOG("Server::Init() - faied to start listening.");
		return false;
	}

	CString address;
	UINT realPort = 0;
	if (mListenSocket.GetSockName(address, realPort))
	{
		LOG("Server::Init() - start listening. %s:%d", address.GetBuffer(0), realPort);
	}

	return true;
}


void Server::Shutdown()
{
	mListenSocket.ShutDown();

	for(int i = 0 ; i != mClientSockets.size() ; ++i)
	{
		delete mClientSockets[i];
	}
}


void Server::OnAccept(int nErrorCode)
{
	if (0 == nErrorCode)
	{
		ClientSocket* socket = new ClientSocket;

		if (mListenSocket.Accept(*socket))
		{
			mClientSockets.push_back(socket);
			socket->ResolvePeerAddress();

			LOG("Server::OnAccept() - accepted. %s", socket->GetPeerAddress());
		}
		else
		{
			delete socket;
			LOG("Server::OnAccept() - failed to accept. error[%d]", GetLastError());
		}
	}
}


void Server::OnClose(ClientSocket* socket)
{
	ASSERT(socket);

	mClientSockets.erase(std::remove(mClientSockets.begin(), mClientSockets.end(), socket));
	delete socket;
}


void Server::OnReceive(ClientSocket* socket, const rapidjson::Document& jsonData)
{
	ASSERT(socket);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	jsonData.Accept(writer);

	// echo json string including null
	socket->PostSend(buffer.GetString(), buffer.Size()+1);
}


