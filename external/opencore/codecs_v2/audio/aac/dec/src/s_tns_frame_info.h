

#ifndef S_TNS_FRAME_INFO_H
#define S_TNS_FRAME_INFO_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*----------------------------------------------------------------------------
    ; INCLUDES
    ----------------------------------------------------------------------------*/
#include "pv_audio_type_defs.h"
#include "e_tns_const.h"
#include "s_tnsfilt.h"

    /*----------------------------------------------------------------------------
    ; MACROS
    ; Define module specific macros here
    ----------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
    ; DEFINES
    ; Include all pre-processor statements here.
    ----------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
    ; EXTERNAL VARIABLES REFERENCES
    ; Declare variables used in this module but defined elsewhere
    ----------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
    ; SIMPLE TYPEDEF'S
    ----------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
    ; ENUMERATED TYPEDEF'S
    ----------------------------------------------------------------------------*/

    /*----------------------------------------------------------------------------
    ; STRUCTURES TYPEDEF'S
    ----------------------------------------------------------------------------*/
    typedef struct
    {
        Bool tns_data_present;

        /* Information about the number of filters for each window. */
        Int n_filt[TNS_MAX_WIN];

        /*
         * Filter Information
         *
         * For short windows, there is a maximum of
         * 1 filter per window (8 total)
         *
         * For long windows, there is a maximum of 3 filters
         *
         */
        TNSfilt filt[TNS_MAX_WIN];

        /*
         * For short windows, there is a maximum of 8 filters,
         * each of order 7 (requring 56 Ints)
         *
         * For long windows, there is a maximum of 3 filters,
         * each of order 20 (requiring 60 Ints)
         *
         * So, 3*TNS_MAX_ORDER declares an array of sufficient
         * size (60) for both cases.
         */
        Int32 lpc_coef[3*TNS_MAX_ORDER];

    } TNS_frame_info;

    /*----------------------------------------------------------------------------
    ; GLOBAL FUNCTION DEFINITIONS
    ; Function Prototype declaration
    ----------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* S_TNS_FRAME_INFO_H */
