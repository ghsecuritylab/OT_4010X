
#ifndef PVMP4FFCN_TYPES_H_INCLUDED
#define PVMP4FFCN_TYPES_H_INCLUDED

/** Port tags */
enum PvmfMp4FFCNPortTag
{
    PVMF_MP4FFCN_PORT_TYPE_SINK = 0
};

/** Enumerated list of errors */
typedef enum
{
    PVMF_MP4FFCN_ERROR_INVALID_MEDIA_DATA = PVMF_NODE_ERROR_EVENT_LAST,
    PVMF_MP4FFCN_ERROR_ADD_SAMPLE_TO_TRACK_FAILED,
    PVMF_MP4FFCN_ERROR_FINALIZE_OUTPUT_FILE_FAILED
} PvmfMp4FFCNError;

#endif // PVMP4FFCN_TYPES_H_INCLUDED


