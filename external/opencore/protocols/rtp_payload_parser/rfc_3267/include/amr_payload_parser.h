
///////////////////////////////////////////////////////////////////////////////
//
// amr_payload_parser.h
//
// Payload parser for AMR RTP format (RFC 3267).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef AMR_PAYLOAD_PARSER_H
#define AMR_PAYLOAD_PARSER_H

#include "payload_parser.h"
#include "virtual_buffer.h"

class AMRPayloadParser : public IPayloadParser
{
    public:
        OSCL_IMPORT_REF AMRPayloadParser();
        OSCL_IMPORT_REF virtual ~AMRPayloadParser();

        OSCL_IMPORT_REF bool Init(mediaInfo* config);
        OSCL_IMPORT_REF PayloadParserStatus Parse(const Payload& inputPacket,
                Oscl_Vector<Payload, OsclMemAllocator>& vParsedPayloads);

        OSCL_IMPORT_REF void Reposition(const bool adjustSequence = false, const uint32 stream = 0, const uint32 seqnum = 0);
        OSCL_IMPORT_REF uint32 GetMinCurrTimestamp();

    protected:
        bool doParseBandwithEff(VirtualBuffer<uint8>& vb, Payload& outputPacket);
        bool doParseOctetAligned(VirtualBuffer<uint8>& vb, Payload& outputPacket);

        enum AMR_TYPE
        {
            AMR_TYPE_NONE,
            AMR_TYPE_AMR,
            AMR_TYPE_WB,

        };

        AMR_TYPE        amr_type;
        uint16          modeSet;
        uint16          channels;
        bool            octetAligned;
        bool            crc;
        bool            interleave;
};

#endif //AMR_PAYLOAD_PARSER_H
