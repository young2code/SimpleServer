#pragma once

#include <boost/circular_buffer.hpp>

class ClientSocket : public CAsyncSocket
{
public:
	ClientSocket();
	virtual ~ClientSocket();

	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnClose(int nErrorCode);

	void ResolvePeerAddress();
	const char* GetPeerAddress() const { return mAddress.c_str(); }

private:
	void GenerateJSON();

private:
	typedef boost::circular_buffer<char> RingBuffer;
	RingBuffer mRecvBuffer;
	RingBuffer mSendBuffer;

	std::string mAddress;
};