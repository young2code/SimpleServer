#include "stdafx.h"

#include "ClientSocket.h"
#include "Server.h"

#include <rapidjson/document.h>
#include <boost/array.hpp>
 
#pragma warning(disable:4996) //4996: 'std::copy': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'

#undef min
#undef max

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

	if (0 == result)
	{
		TRACE("ClientSocket::OnReceive - closed. [%s]", mAddress.c_str());
		Close();
		return;
	}

	if (SOCKET_ERROR == result)
	{
		if (WSAEWOULDBLOCK != GetLastError())
		{
			TRACE("ClientSocket::OnReceive - receive error[%d]. [%s]", GetLastError(), mAddress.c_str());
			Close();
		}
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
	if (0 != nErrorCode)
	{
		TRACE("ClientSocket::OnSend - error[%d]. [%s]", nErrorCode, mAddress.c_str());

		Close();
		return;
	}

	while (!mSendBuffer.empty())
	{
		boost::array<char, kMaxDataSize> jsonStr;

		RingBuffer::iterator itorEnd = mSendBuffer.begin();
		size_t total = std::min<size_t>(mSendBuffer.size(), kMaxDataSize);
		std::advance(itorEnd, total);

		ASSERT(std::distance(mRecvBuffer.begin(), itorEnd) <= kMaxDataSize);
		std::copy(mSendBuffer.begin(), itorEnd, jsonStr.begin());

		int result = Send(jsonStr.data(), total);

		if (SOCKET_ERROR == result)
		{
			if (WSAEWOULDBLOCK != GetLastError())
			{
				TRACE("ClientSocket::OnSend - send error[%d]. [%s]", GetLastError(), mAddress.c_str());
				Close();
			}

			return;
		}
		
		ASSERT(result > 0);

		TRACE("ClientSocket::OnSend - sending succeeded. %d bytes. %s", result, mAddress.c_str());

		itorEnd = mSendBuffer.begin();
		std::advance(itorEnd, result);
		mSendBuffer.erase(mSendBuffer.begin(), itorEnd);
	}
}


void ClientSocket::OnClose(int nErrorCode)
{
	TRACE("ClientSocket::OnClose - [%s]", mAddress.c_str());
	Server::Instance()->OnClose(this);
}


void ClientSocket::PostSend(const char* jsonStr, int total)
{
	int result = 0;

	if (mSendBuffer.empty())
	{
		result = Send(jsonStr, total);

		if (SOCKET_ERROR == result)
		{
			if (WSAEWOULDBLOCK != GetLastError())
			{
				TRACE("ClientSocket::PostSend - send error[%d]. [%s]", GetLastError(), mAddress.c_str());
				Close();
				return;
			}

			result = 0;
		}
	}

	if (result >= 0 && result < total)
	{
		mSendBuffer.insert(mSendBuffer.end(), jsonStr + result, jsonStr + total); 
	}
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
