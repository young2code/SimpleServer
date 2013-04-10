#include "stdafx.h"

#include "Server.h"

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
		TRACE("Server::Init() - start listening.  %S:%d", address.GetBuffer(0), realPort);
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
		ClientSocket* pSocket = new ClientSocket;
		mClientSockets.push_back(pSocket);

		mListenSocket.Accept(*pSocket);
	}
}
