
packet myPacket extends cPacket
{
    int sourceAddress;
    int destAddress;
    unsigned int length;
    uint64_t time;
    uint8_t Payload[2048];
};