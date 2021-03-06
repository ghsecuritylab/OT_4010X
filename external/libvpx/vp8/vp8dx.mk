include $(SRC_PATH_BARE)/$(VP8_PREFIX)vp8_common.mk

VP8_DX_EXPORTS += exports_dec

VP8_DX_SRCS-yes += $(VP8_COMMON_SRCS-yes)
VP8_DX_SRCS-no  += $(VP8_COMMON_SRCS-no)
VP8_DX_SRCS_REMOVE-yes += $(VP8_COMMON_SRCS_REMOVE-yes)
VP8_DX_SRCS_REMOVE-no  += $(VP8_COMMON_SRCS_REMOVE-no)

ifeq ($(ARCH_ARM),yes)
  include $(SRC_PATH_BARE)/$(VP8_PREFIX)vp8dx_arm.mk
endif

VP8_DX_SRCS-yes += vp8_dx_iface.c

CFLAGS+=-I$(SRC_PATH_BARE)/$(VP8_PREFIX)decoder


# common
#define ARM
#define DISABLE_THREAD

#INCLUDES += algo/vpx_common/vpx_mem/include
#INCLUDES += common
#INCLUDES += common
#INCLUDES += common
#INCLUDES += common
#INCLUDES += decoder



# decoder
#define ARM
#define DISABLE_THREAD

#INCLUDES += algo/vpx_common/vpx_mem/include
#INCLUDES += common
#INCLUDES += common
#INCLUDES += common
#INCLUDES += common
#INCLUDES += decoder

VP8_DX_SRCS-yes += decoder/dboolhuff.c
VP8_DX_SRCS-yes += decoder/decodemv.c
VP8_DX_SRCS-yes += decoder/decodframe.c
VP8_DX_SRCS-yes += decoder/dequantize.c
VP8_DX_SRCS-yes += decoder/detokenize.c
VP8_DX_SRCS-yes += decoder/generic/dsystemdependent.c
VP8_DX_SRCS-yes += decoder/dboolhuff.h
VP8_DX_SRCS-yes += decoder/decodemv.h
VP8_DX_SRCS-yes += decoder/decoderthreading.h
VP8_DX_SRCS-yes += decoder/dequantize.h
VP8_DX_SRCS-yes += decoder/detokenize.h
VP8_DX_SRCS-yes += decoder/onyxd_int.h
VP8_DX_SRCS-yes += decoder/treereader.h
VP8_DX_SRCS-yes += decoder/onyxd_if.c
VP8_DX_SRCS-yes += decoder/threading.c
VP8_DX_SRCS-yes += decoder/idct_blk.c
VP8_DX_SRCS-$(CONFIG_MULTITHREAD) += decoder/reconintra_mt.h
VP8_DX_SRCS-$(CONFIG_MULTITHREAD) += decoder/reconintra_mt.c

VP8_DX_SRCS-yes := $(filter-out $(VP8_DX_SRCS_REMOVE-yes),$(VP8_DX_SRCS-yes))

VP8_DX_SRCS-$(ARCH_X86)$(ARCH_X86_64) += decoder/x86/dequantize_x86.h
VP8_DX_SRCS-$(ARCH_X86)$(ARCH_X86_64) += decoder/x86/x86_dsystemdependent.c
VP8_DX_SRCS-$(HAVE_MMX) += decoder/x86/dequantize_mmx.asm
VP8_DX_SRCS-$(HAVE_MMX) += decoder/x86/idct_blk_mmx.c
VP8_DX_SRCS-$(HAVE_SSE2) += decoder/x86/idct_blk_sse2.c
