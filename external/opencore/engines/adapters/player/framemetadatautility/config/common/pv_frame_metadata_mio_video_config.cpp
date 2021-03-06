
#include "pv_frame_metadata_mio_video.h"
#include "pvlogger.h"

// Add/remove header file and modify function below for different CC support
#include "cczoomrotation16.h"


PVMFStatus PVFMVideoMIO::CreateYUVToRGBColorConverter(ColorConvertBase*& aCC, PVMFFormatType aRGBFormatType)
{
    int32 leavecode = 0;
    if (aRGBFormatType == PVMF_MIME_RGB16)
    {
        OSCL_TRY(leavecode, aCC = ColorConvert16::NewL());
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CreateYUVToRGBColorConverter() Unsupported RGB mode for color converter. Asserting"));
        return PVMFErrNotSupported;
    }

    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CreateYUVToRGBColorConverter() Color converter instantiation did a leave"));
                         return PVMFErrNoResources;
                        );

    return PVMFSuccess;
}


PVMFStatus PVFMVideoMIO::DestroyYUVToRGBColorConverter(ColorConvertBase*& aCC, PVMFFormatType aRGBFormatType)
{
    OSCL_ASSERT(aCC != NULL);

    if (aRGBFormatType == PVMF_MIME_RGB16)
    {
        OSCL_DELETE(((ColorConvert16*)aCC));
        aCC = NULL;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVFMVideoMIO::CreateYUVToRGBColorConverter() Unsupported RGB mode for color converter. Asserting"));
        return PVMFErrNotSupported;
    }

    return PVMFSuccess;
}


