

#ifndef _NAS_UTRANCTRL_FSM_SWITCH_ON_TBL_H_
#define _NAS_UTRANCTRL_FSM_SWITCH_ON_TBL_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/

#include  "vos.h"
#include  "NasFsm.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
/*****************************************************************************
  1 全局变量定义
*****************************************************************************/
extern NAS_STA_STRU                            g_astNasUtranCtrlSwitchOnStaTbl[];


/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define NAS_UTRANCTRL_GetSwitchOnStaTbl()      (g_astNasUtranCtrlSwitchOnStaTbl)


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


enum NAS_UTRANCTRL_SWITCH_ON_STA_ENUM
{
    /* 开机状态机，初始状态 */
    NAS_UTRANCTRL_SWITCH_ON_STA_INIT,    

    /* 开机初始化过程,等待WAS的回复 */
    NAS_UTRANCTRL_SWITCH_ON_STA_WAIT_WAS_START_CNF,

    /* 开机初始化过程,等待TD的回复 */
    NAS_UTRANCTRL_SWITCH_ON_STA_WAIT_TD_START_CNF,

    NAS_UTRANCTRL_SWITCH_ON_BUTT
};

typedef VOS_UINT32  NAS_UTRANCTRL_SWITCH_ON_STA_ENUM_UINT32;


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/



/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_UINT32 NAS_UTRANCTRL_GetSwitchOnStaTblSize( VOS_VOID  );

NAS_FSM_DESC_STRU * NAS_UTRANCTRL_GetSwitchOnFsmDescAddr(VOS_VOID);

#endif  /* #if (FEATURE_ON == FEATURE_UE_MODE_TDS) */


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
