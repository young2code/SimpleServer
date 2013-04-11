#include "stdafx.h"

#include "ClientSocket.h"
#include "Server.h"

#include <rapidjson/document.h>
#include <boost/array.hpp>
 
#pragma warning(disable:4996) //4996: 'std::copy': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'

namespace
{
	const int kMaxDataSize = 1024;
};

ClientSocket::ClientSocket()
	: mRecvBuffer(kMaxDataSize)
	, mSendBuffer(kMaxDataSize)
{
}


ClientSocket::~ClientSocket()
{
}

	
void ClientSocket::ResolvePeerAddress()
{
	CString address;
	UINT port = 0;
	GetPeerName(address, port);
	address.AppendFormat(":%d", port);
	mAddress = address;
}


void ClientSocket::OnReceive(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		TRACE("ClientSocket::OnReceive - error[%d]. [%s]", nErrorCode, mAddress.c_str());

		Close();
		return;
	}

	char temp[kMaxDataSize];
	int result = Receive(temp, sizeof(temp));

	if (result == 0)
	{
		TRACE("ClientSocket::OnReceive - closed. [%s]", mAddress.c_str());
		Close();
		return;
	}

	if (result == SOCKET_ERROR && result != WSAEWOULDBLOCK)
	{
		TRACE("ClientSocket::OnReceive - error[%d]. [%s]", result, mAddress.c_str());
		Close();
		return;
	}

	int available = mRecvBuffer.capacity() - mRecvBuffer.size();
	if (available < result)
	{
		mRecvBuffer.set_capacity(mRecvBuffer.capacity() + result*2);
	}
	ASSERT(mRecvBuffer.capacity() - mRecvBuffer.size() >= static_cast<size_t>(result));

	mRecvBuffer.insert(mRecvBuffer.end(), temp, temp + result);

	TRACE("ClientSocket::OnReceive - received [%d]. [%s]", result, mAddress.c_str());

	GenerateJSON();
}


void ClientSocket::OnSend(int nErrorCode)
{
}


void ClientSocket::OnClose(int nErrorCode)
{
	TRACE("ClientSocket::OnClose - [%s]", mAddress.c_str());
	Server::Instance()->OnClose(this);
}


void ClientSocket::GenerateJSON()
{
	RingBuffer::iterator itorEnd = std::find(mRecvBuffer.begin(), mRecvBuffer.end(), '\0');

	while (itorEnd != mRecvBuffer.end())
	{
		++itorEnd;

		boost::array<char, kMaxDataSize> jsonStr;

		ASSERT(std::distance(mRecvBuffer.begin(), itorEnd) <= kMaxDataSize);
		std::copy(mRecvBuffer.begin(), itorEnd, jsonStr.begin());

		mRecvBuffer.erase(mRecvBuffer.begin(), itorEnd);

		rapidjson::Document jsonData;
		jsonData.Parse<0>(jsonStr.data());

		if (jsonData.HasParseError())
		{
			TRACE("ClientSocket::GenerateJSON - parsing failed. %s error[%s], %s", jsonStr.data(), jsonData.GetParseError(), mAddress.c_str());
		}
		else
		{
			TRACE("ClientSocket::GenerateJSON - parsing succeeded. %s, %s", jsonStr.data(), mAddress.c_str());

			Server::Instance()->OnReceive(this, jsonData);
		}

		itorEnd = std::find(mRecvBuffer.begin(), mRecvBuffer.end(), '\0');
	}
}
