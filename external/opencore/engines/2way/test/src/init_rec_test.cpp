
#include "init_rec_test.h"



void init_rec_test::test()
{
    fprintf(fileoutput, "Start init rec test, num runs %d, proxy %d.\n", iMaxRuns, iUseProxy);
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


void init_rec_test::Run()
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

void init_rec_test::DoCancel()
{
}


void init_rec_test::HandleInformationalEventL(const CPVCmnAsyncInfoEvent& aEvent)
{
}

void init_rec_test::CommandCompletedL(const CPVCmnCmdResp& aResponse)
{
    int error = 0;
    switch (aResponse.GetCmdType())
    {
        case PVT_COMMAND_INIT:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_wHeapString<OsclMemAllocator> filename(RECORDED_CALL_FILENAME);
                OSCL_TRY(error, terminal->InitRecordFileL(filename));
                if (error)
                {
                    test_is_true(false);
                    reset();
                }
            }
            else
            {
                test_is_true(false);
                reset();
            }
            break;

        case PVT_COMMAND_RESET:
            RunIfNotReady();
            break;

        case PVT_COMMAND_INIT_RECORD_FILE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                OSCL_TRY(error, terminal->ResetRecordFileL());
                if (error)
                {
                    test_is_true(false);
                    reset();
                }
                else
                {
                    if (iCurrentRun >= (iMaxRuns - 1))
                    {
                        test_is_true(true);
                    }
                }
            }
            else
            {
                test_is_true(false);
                reset();
            }
            break;

        case PVT_COMMAND_RESET_RECORD_FILE:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                iCurrentRun++;
                if (iCurrentRun < iMaxRuns)
                {
                    OSCL_wHeapString<OsclMemAllocator> filename(RECORDED_CALL_FILENAME);
                    OSCL_TRY(error, terminal->InitRecordFileL(filename));
                    if (error)
                    {
                        test_is_true(false);
                        reset();
                    }
                }
                else
                {
                    OSCL_TRY(error, terminal->ResetL());
                    if (error)
                    {
                        test_is_true(false);
                        RunIfNotReady();
                    }
                }
            }
            else
            {
                test_is_true(false);
                RunIfNotReady();
            }
            break;
    }
}

bool init_rec_test::start_async_test()
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





