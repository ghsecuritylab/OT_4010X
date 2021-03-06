
#include "video_only_pause_test.h"



void video_only_pause_test::test()
{
    fprintf(fileoutput, "Start video only pause test, num runs %d, proxy %d.\n", iMaxRuns, iUseProxy);
    int error = 0;

    scheduler = OsclExecScheduler::Current();

    this->AddToScheduler();

    if (start_async_test())
    {
        OSCL_TRY(error, scheduler->StartScheduler());
        if (error != 0)
        {
            OSCL_LEAVE(error);
        }
    }

    destroy_sink_source();

    this->RemoveFromScheduler();
}


void video_only_pause_test::Run()
{
    if (terminal)
    {
        if (iUseProxy)
        {
            CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }
        terminal = NULL;
    }

    scheduler->StopScheduler();
}

void video_only_pause_test::DoCancel()
{
}


void video_only_pause_test::HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent)
{
    int error = 0;

    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_INCOMING_TRACK:
            TPVChannelId id;

            if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[0] == PV_VIDEO)
            {
                OSCL_TRY(error, iVideoAddSinkId = terminal->AddDataSinkL(*iVideoSink, id));
                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }
            }
            break;

        case PVT_INDICATION_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            break;

        case PVT_INDICATION_CLOSE_TRACK:
            if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[0] == PV_VIDEO)
            {
                if (((CPVCmnAsyncEvent&)aEvent).GetLocalBuffer()[1] == INCOMING)
                {
                    iVideoSinkAdded = false;
                }
                else
                {
                    iVideoSourceAdded = false;
                }
            }
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        default:
            break;
    }
}

void video_only_pause_test::CommandCompletedL(const CPVCmnCmdResp& aResponse)
{
    int error = 0;
    switch (aResponse.GetCmdType())
    {
        case PVT_COMMAND_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, terminal->ConnectL(iConnectOptions));
                if (error)
                {
                    test_is_true(false);
                    reset();
                }
                else
                {
                }
            }
            else
            {
                test_is_true(false);
                RunIfNotReady();
            }
            break;

        case PVT_COMMAND_RESET:
            RunIfNotReady();
            break;

        case PVT_COMMAND_ADD_DATA_SOURCE:
            if (aResponse.GetCmdId() == iVideoAddSourceId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoSourceAdded = true;

                    OSCL_TRY(error, iVideoPauseSourceId = terminal->PauseL(*iVideoSource));
                    if (error)
                    {
                        test_is_true(false);
                        disconnect();
                    }
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                }
                iVideoAddSourceId = 0;
            }
            break;

        case PVT_COMMAND_REMOVE_DATA_SOURCE:
            if (aResponse.GetCmdId() == iVideoRemoveSourceId)
            {
                iVideoRemoveSourceId = 0;
                iVideoSourceAdded = false;
            }
            break;

        case PVT_COMMAND_ADD_DATA_SINK:
            if (aResponse.GetCmdId() == iVideoAddSinkId)
            {
                if (aResponse.GetCmdStatus() == PVMFSuccess)
                {
                    iVideoSinkAdded = true;

                    OSCL_TRY(error, iVideoPauseSinkId = terminal->PauseL(*iVideoSink));
                    if (error)
                    {
                        test_is_true(false);
                        disconnect();
                    }
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                }
                iVideoAddSinkId = 0;
            }
            break;

        case PVT_COMMAND_REMOVE_DATA_SINK:
            if (aResponse.GetCmdId() == iVideoRemoveSinkId)
            {
                iVideoRemoveSinkId  = 0;
                iVideoSinkAdded = false;
            }
            break;

        case PVT_COMMAND_CONNECT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, iVideoAddSourceId = terminal->AddDataSourceL(*iVideoSource));
                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }
            }
            else
            {
                test_is_true(false);
                reset();
            }
            break;

        case PVT_COMMAND_DISCONNECT:
            iAudioSourceAdded = false;
            iVideoSourceAdded = false;
            iAudioSinkAdded = false;
            iVideoSinkAdded = false;
            reset();
            break;

        case PVT_COMMAND_PAUSE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aResponse.GetCmdId() == iVideoPauseSourceId)
                {
                    OSCL_TRY(error, iVideoResumeSourceId = terminal->ResumeL(*iVideoSource));
                }
                else if (aResponse.GetCmdId() == iVideoPauseSinkId)
                {
                    OSCL_TRY(error, iVideoResumeSinkId = terminal->ResumeL(*iVideoSink));
                }
                else
                {
                    error = -1;
                }

                if (error)
                {
                    test_is_true(false);
                    disconnect();
                }
            }
            else
            {
                test_is_true(false);
                disconnect();
            }
            break;

        case PVT_COMMAND_RESUME:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aResponse.GetCmdId() == iVideoResumeSourceId)
                {
                    iVideoSourceResumed = true;
                }
                else if (aResponse.GetCmdId() == iVideoResumeSinkId)
                {
                    iVideoSinkResumed = true;
                }
                else
                {
                    test_is_true(false);
                    disconnect();
                    break;
                }

                if (check_video_resumed())
                {
                    iCurrentRun++;

                    if (iCurrentRun < iMaxRuns)
                    {
                        OSCL_TRY(error, iVideoPauseSourceId = terminal->PauseL(*iVideoSource));
                        if (error)
                        {
                            test_is_true(false);
                            disconnect();
                        }
                        else
                        {
                            OSCL_TRY(error, iVideoPauseSinkId = terminal->PauseL(*iVideoSink));
                            if (error)
                            {
                                test_is_true(false);
                                disconnect();
                            }
                        }
                        iVideoSourceResumed = false;
                        iVideoSinkResumed = false;
                    }
                    else
                    {
                        test_is_true(true);
                        disconnect();
                    }
                }
            }
            else
            {
                test_is_true(false);
                disconnect();
            }
            break;

        case PVT_COMMAND_CANCEL_ALL_COMMANDS:
            break;
    }
}

bool video_only_pause_test::start_async_test()
{
    int error = 0;

    if (iUseProxy)
    {
        OSCL_TRY(error, terminal = CPV2WayProxyFactory::CreateTerminalL(PV_324M,
                                   (MPVCmnCmdStatusObserver *) this,
                                   (MPVCmnInfoEventObserver *) this,
                                   (MPVCmnErrorEventObserver *) this));
    }
    else
    {
        OSCL_TRY(error, terminal = CPV2WayEngineFactory::CreateTerminalL(PV_324M,
                                   (MPVCmnCmdStatusObserver *) this,
                                   (MPVCmnInfoEventObserver *) this,
                                   (MPVCmnErrorEventObserver *) this));
    }

    if (error)
    {
        test_is_true(false);
        return false;
    }

    create_sink_source();

    OSCL_TRY(error, terminal->InitL(iSdkInitInfo, iCommServer));
    if (error)
    {
        test_is_true(false);
        if (iUseProxy)
        {
            CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }
        terminal = NULL;
        return false;
    }

    return true;
}





