#ifndef __BUFFER_PROTOCOL_H__
#define __BUFFER_PROTOCOL_H__

#include "Protocol.h"

class BufferProtocol : public Protocol
{
public:
	BufferProtocol(int32_t interface_id, uint32_t max_package_size);
	~BufferProtocol();

	// Inherited via Protocol
	virtual int PushBuffer(uint8_t * points, int32_t len) override;
	virtual ProtocolType GetProtocolType() { return PT_BUFFER; };
	virtual const uint8_t * PackageBuffer(uint8_t * points, int32_t in_len, int32_t * out_len) override;

};

#endif // !__BUFFER_PROTOCOL_H__