#include "stdafx.h"

#include "Server.h"
#include <rapidjson/document.h>

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
		TRACE("Server::Init() - faied to create a listener socket.");
		return false;
	}

	BOOL reuse = TRUE;
	mListenSocket.SetSockOpt(SO_REUSEADDR, &reuse, sizeof(BOOL));

	if (!mListenSocket.Listen())
	{
		TRACE("Server::Init() - faied to start listening.");
		return false;
	}

	CString address;
	UINT realPort = 0;
	if (mListenSocket.GetSockName(address, realPort))
	{
		TRACE("Server::Init() - start listening. %s:%d", address.GetBuffer(0), realPort);
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

			TRACE("Server::OnAccept() - accepted. %s", socket->GetPeerAddress());
		}
		else
		{
			delete socket;
			TRACE("Server::OnAccept() - failed to accept. error[%d]", GetLastError());
		}
	}
}


void Server::OnClose(ClientSocket* socket)
{
	mClientSockets.erase(std::remove(mClientSockets.begin(), mClientSockets.end(), socket));
	delete socket;
}


void Server::OnReceive(ClientSocket* socket, const rapidjson::Document& jsonData)
{
	/* do whatever we want with jsonData. */
}


