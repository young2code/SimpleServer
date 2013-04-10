#include "stdafx.h"

#include "ListenSocket.h"
#include "Server.h"

ListenSocket::ListenSocket()
{
}


ListenSocket::~ListenSocket()
{
}

void ListenSocket::OnAccept(int nErrorCode)
{
	Server::Instance()->OnAccept(nErrorCode);
}
