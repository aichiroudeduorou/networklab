#include "stdafx.h"
#include "SRRdtReceiver.h"
#include "utils.h"

SRRdtReceiver::SRRdtReceiver(int n, int seqNumBits):
    MAX_SEQ((seqNumBits > 0 && seqNumBits <= 16) ? (1 << seqNumBits) : (1 << 16)),
    N(n),
    base(0)
{
}

SRRdtReceiver::~SRRdtReceiver()
{
}

inline bool SRRdtReceiver::inWindow(int seqNum) {
    int start = base;
    int end = (base + N - 1) % MAX_SEQ;
    if (start < end) {
        return start <= seqNum && seqNum <= end;
    } else {
		return start <= seqNum || seqNum <= end;
    }
}

inline bool SRRdtReceiver::inPrevWindow(int seqNum) {
    int start = base - N;
    if (start < 0) start += MAX_SEQ;
    int end = base - 1;
    if (end < 0) end += MAX_SEQ;
    if (start < end) {
        return start <= seqNum && seqNum <= end;
    } else {
        return start <= seqNum || seqNum <= end;
    }
}

void SRRdtReceiver::receive(const Packet &packet) {
    int checkSum = pUtils->calculateCheckSum(packet);
    if (checkSum == packet.checksum) {
        pUtils->printPacket("接收方正确收到发送方的报文", packet);
        printf("接收方窗口：");
        for (int i = base; i != (base + N) % MAX_SEQ; i = (i + 1) % MAX_SEQ) {
            if (cache.count(i))
                printf("%d ", i);
            else if(i==packet.seqnum)
                printf("%d ", i);
        }
        printf("\n");
        if (inWindow(packet.seqnum)) {
            Packet ackPkt = makeAckPkt(packet.seqnum);
	        pns->sendToNetworkLayer(SENDER, ackPkt);
            if (!cache.count(packet.seqnum)) {  // 不在缓存区中
                if (packet.seqnum == base) {
                    Message msg;
                    memcpy(msg.data, packet.payload, Configuration::PAYLOAD_SIZE);
                    pns->delivertoAppLayer(RECEIVER, msg);
                    base = (base + 1) % MAX_SEQ;
                    while (cache.count(base)) {
                        memcpy(msg.data, cache[base].payload, Configuration::PAYLOAD_SIZE);
                        pns->delivertoAppLayer(RECEIVER, msg);
                        cache.erase(base);
                        base = (base + 1) % MAX_SEQ;
                    }
                } else {
                    cache[packet.seqnum] = packet;
                }
            }
        } else if (inPrevWindow(packet.seqnum)) {
            Packet ackPkt = makeAckPkt(packet.seqnum);
	        pns->sendToNetworkLayer(SENDER, ackPkt);
        }
    } else {
        pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
    }
}