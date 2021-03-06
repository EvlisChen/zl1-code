
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "NasMmlCtx.h"
#include "NasMmcCtx.h"
#include "NasMmcProcNvim.h"
#include "NasComm.h"
#include "UsimmApi.h"
#include "NasMmlLib.h"
#include "NasMmcProcUsim.h"
#include "pscfg.h"
#include "NasMmcSndOm.h"
#include "NasUsimmApi.h"
#if (FEATURE_LTE == FEATURE_ON)
#include "msp_nvim.h"
#include "LNvCommon.h"
#include "LNasNvInterface.h"
#include "LPsNvInterface.h"
#include "ImsaNvInterface.h"

#include "NasMmcSndLmm.h"
#endif

#include "ScInterface.h"
#include "NasNvInterface.h"
#include "TafNvInterface.h"


/* DSDS使能NV从TTF读取 */
#include "TtfNvInterface.h"
#include "WasNvInterface.h"

#include "NasMmcComFunc.h"

/* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
#include "SysNvId.h"
/* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define    THIS_FILE_ID        PS_FILE_ID_NAS_MMC_PROC_NIVM_C

/*****************************************************************************
  2 全局变量
*****************************************************************************/

/*****************************************************************************
  3 宏定义
*****************************************************************************/


VOS_VOID NAS_MMC_ReadCsOnlyDataServiceSupportNvim( VOS_VOID )
{
    NAS_MML_CS_ONLY_DATA_SERVICE_SUPPORT_FLG_STRU           stCsOnlyDataServiceSupportFlg;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU                    *pstMiscellaneousCfgInfo = VOS_NULL_PTR;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    PS_MEM_SET(&stCsOnlyDataServiceSupportFlg, 0, sizeof(stCsOnlyDataServiceSupportFlg));
    if (NV_OK != NV_Read(en_NV_Item_CS_ONLY_DATA_SERVICE_SUPPORT_FLG,
                         &stCsOnlyDataServiceSupportFlg, 
                         sizeof(stCsOnlyDataServiceSupportFlg)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadCsOnlyDataServiceSupportNvim():WARNING: en_NV_Item_CS_ONLY_DATA_SERVICE_SUPPORT_FLG Error");

        return;
    }


    if (NAS_MMC_NV_ITEM_ACTIVE != stCsOnlyDataServiceSupportFlg.ucActiveFlg)
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadCsOnlyDataServiceSupportNvim(): NV deactive.");
        return;
    }

    if (0 != stCsOnlyDataServiceSupportFlg.ucCsOnlyDataServiceSupportFlg)
    {
        stCsOnlyDataServiceSupportFlg.ucCsOnlyDataServiceSupportFlg = VOS_TRUE;
    }
    
    pstMiscellaneousCfgInfo->ucCsOnlyDataServiceSupportFlg
             = stCsOnlyDataServiceSupportFlg.ucCsOnlyDataServiceSupportFlg;

    return;             

}


VOS_VOID NAS_MMC_ReadLteDisabledUseLteInfoFlagNvim(VOS_VOID)
{
    NAS_MMC_LTE_DISABLED_USE_LTE_INFO_FLAG_STRU             stLDisabledUseLInfoFlag;
    VOS_UINT32                                              ulLength;

    ulLength                = 0;
    PS_MEM_SET(&stLDisabledUseLInfoFlag, 0, sizeof(stLDisabledUseLInfoFlag));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_LTE_DISABLED_USE_LTE_INFO_FLAG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_LTE_DISABLED_USE_LTE_INFO_FLAG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteDisabledUseLteInfoFlagNvim(): nv length Error");
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_LTE_DISABLED_USE_LTE_INFO_FLAG,
                         &stLDisabledUseLInfoFlag, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadLteDisabledUseLteInfoFlagNvim():WARNING: NV read Error");

        return;
    }

    /* 测试卡nv项默认开启*/
    if ((VOS_TRUE == NAS_USIMMAPI_IsTestCard())
     || (VOS_TRUE == stLDisabledUseLInfoFlag.ucLDisabledRauUseLInfoFlag))
    {
        NAS_MML_SetLteDisabledRauUseLteInfoFlag(VOS_TRUE);
    }

    return;
}



VOS_VOID NAS_MMC_ReadWgRfMainBandNvim( VOS_VOID )
{
    NAS_NVIM_WG_RF_MAIN_BAND_STRU       stRfMainBand;
    VOS_UINT32                          ulDataLen;

    ulDataLen = 0;

    PS_MEM_SET(&stRfMainBand, 0x00, sizeof(stRfMainBand));

    NV_GetLength(en_NV_Item_WG_RF_MAIN_BAND, &ulDataLen);

    if (NV_OK != NV_Read(en_NV_Item_WG_RF_MAIN_BAND, &stRfMainBand, ulDataLen))
    {
        /*打印未成功初始化值，原值保留*/
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadWgRfMainBandNvim():Read en_NV_Item_WG_RF_MAIN_BAND Failed!");
        return;
    }

    NAS_MML_SetGsmBandCapability(stRfMainBand.ulGband);    

    return;
}
VOS_VOID NAS_MMC_ReadRoamingRejectNoRetryFlgNvim( VOS_VOID )
{
    NAS_MML_ROAMINGREJECT_NORETYR_CFG_STRU                 *pstNoRetryCfg = VOS_NULL_PTR;  
    NAS_NVIM_ROAMINGREJECT_NORETYR_CFG_STRU                 stRoamingRejectNoRetryCfg;
    VOS_UINT32                                              ulLength;
    VOS_UINT32                                              i;

    ulLength        = 0;
    i               = 0;

    PS_MEM_SET(&stRoamingRejectNoRetryCfg, 0, sizeof(NAS_NVIM_ROAMINGREJECT_NORETYR_CFG_STRU));
    
    NV_GetLength(en_NV_Item_ROAMING_REJECT_NORETRY_CFG, &ulLength);
    if (ulLength > sizeof(NAS_NVIM_ROAMINGREJECT_NORETYR_CFG_STRU))
    {
        return;
    }

    pstNoRetryCfg = NAS_MML_GetRoamingRejectNoRetryCfg();

    if (NV_OK != NV_Read(en_NV_Item_ROAMING_REJECT_NORETRY_CFG,
                         &stRoamingRejectNoRetryCfg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadRoamingRejectNoRetryFlgNvim():en_NV_Item_ROAMING_REJECT_NORETRY_CFG Error");

        return;
    }

    pstNoRetryCfg->ucNoRetryRejectCauseNum = stRoamingRejectNoRetryCfg.ucNoRetryRejectCauseNum;

    if (pstNoRetryCfg->ucNoRetryRejectCauseNum > NAS_MML_MAX_ROAMING_REJECT_NO_RETRY_CAUSE_NUM)
    {
        pstNoRetryCfg->ucNoRetryRejectCauseNum = NAS_MML_MAX_ROAMING_REJECT_NO_RETRY_CAUSE_NUM;
    }

    for (i = 0; i < pstNoRetryCfg->ucNoRetryRejectCauseNum; i++)
    {
        pstNoRetryCfg->aucNoRetryRejectCause[i] = stRoamingRejectNoRetryCfg.aucNoRetryRejectCause[i];
    }

    return;

}

VOS_VOID NAS_MMC_ReadImeisvNvim( VOS_VOID )
{
    VOS_UINT8                           aucImeisv[NV_ITEM_IMEI_SIZE];
    VOS_UINT8                           i;
    NAS_MML_MS_CAPACILITY_INFO_STRU    *pstMsCapability = VOS_NULL_PTR;
    VOS_UINT8                           aucImeiSvn[4];

    MODEM_ID_ENUM_UINT16                enModemId;


    PS_MEM_SET(aucImeisv, 0x00, sizeof(aucImeisv));

    pstMsCapability = NAS_MML_GetMsCapability();

    /* 由PID获取MODEMID */
    enModemId = VOS_GetModemIDFromPid(WUEPS_PID_MMC);

    /* 调用SC接口读取IMEI号码 */
    SC_PERS_NvRead(enModemId, en_NV_Item_IMEI, aucImeisv, NV_ITEM_IMEI_SIZE);

    for (i = 0; i < NV_ITEM_IMEI_SIZE; i++)
    {
        pstMsCapability->aucImeisv[i] = aucImeisv[i];
    }

    /* 需要将最后两个字节置为0 */
    pstMsCapability->aucImeisv[NV_ITEM_IMEI_SIZE-2] = 0;
    pstMsCapability->aucImeisv[NV_ITEM_IMEI_SIZE-1] = 0;



    PS_MEM_SET(aucImeiSvn, 0x00, sizeof(aucImeiSvn));

    /* 后获取IMEISV的内容 */
    if (NV_OK != NV_Read(en_NV_Item_Imei_Svn,
                    aucImeiSvn, sizeof(aucImeiSvn)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImeisvNvim:Read Nvim Failed");
        return;

    }

    /* NV项有效,则更新 */
    if (VOS_TRUE == aucImeiSvn[0])
    {
         pstMsCapability->aucImeisv[NV_ITEM_IMEI_SIZE-2] = aucImeiSvn[1];
         pstMsCapability->aucImeisv[NV_ITEM_IMEI_SIZE-1] = aucImeiSvn[2];
    }

    return;
}
VOS_VOID NAS_MMC_ReadSupported3GppReleaseNvim(VOS_VOID)
{
    NAS_MML_MS_3GPP_REL_STRU           *pstMs3GppRel = VOS_NULL_PTR;
    NAS_MMC_NVIM_MSCR_VERSION_STRU      stMscrVersion;
    NAS_MMC_NVIM_SGSNR_VERSION_STRU     stSgsnrVersion;
    VOS_UINT32                          ulLength;

    NAS_MMC_NVIM_SUPPORT_3GPP_RELEASE_STRU  stSupported3GppRelease;

    ulLength                                          = 0;


    stSupported3GppRelease.aucSupported3GppRelease[0] = NAS_MML_3GPP_REL_R4 + 1;
    stSupported3GppRelease.aucSupported3GppRelease[1] = NAS_MML_3GPP_REL_R6 + 1;

    stMscrVersion.ucStatus          = NAS_MMC_NV_ITEM_DEACTIVE;
    stMscrVersion.ucUeMscrVersion   = 0;

    stSgsnrVersion.ucStatus         = NAS_MMC_NV_ITEM_DEACTIVE;
    stSgsnrVersion.ucUeSgsnrVersion = 0;

    pstMs3GppRel = NAS_MML_GetMs3GppRel();

    /* 读取支持的协议栈版本 */
    if(NV_OK != NV_Read (en_NV_Item_NAS_Supported_3GPP_Release,
                         &stSupported3GppRelease, NAS_MMC_NVIM_SUPPORTED_3GPP_RELEASE_SIZE))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSupported3GppReleaseNvim(): en_NV_Item_Supported_3GPP_Release Error");
        return;
    }

    /* NV手册中描述NV中记录值含义:
    1：R4版本；
    2：R5版本；
    3：R6版本（默认值）；
    4：R7版本。
    */

    pstMs3GppRel->enMsGsmRel   = stSupported3GppRelease.aucSupported3GppRelease[0] - 1;
    pstMs3GppRel->enMsWcdmaRel = stSupported3GppRelease.aucSupported3GppRelease[1] - 1;
    NV_GetLength(en_NV_Item_UE_MSCR_VERSION, &ulLength);
    /* 读取MSC的版本 */
    if (NV_OK != NV_Read (en_NV_Item_UE_MSCR_VERSION, &stMscrVersion, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSupported3GppReleaseNvim():WARNING: NV_Read en_NV_Item_UE_MSCR_VERSION Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stMscrVersion.ucStatus)
    {
        pstMs3GppRel->enMsMscRel = stMscrVersion.ucUeMscrVersion;
    }


    /* 读取SGSN的版本 */
    if (NV_OK != NV_Read (en_NV_Item_UE_SGSNR_VERSION, &stSgsnrVersion, sizeof(stSgsnrVersion)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSupported3GppReleaseNvim():WARNING: NV_Read en_NV_Item_UE_SGSNR_VERSION Error");
        return;
    }

    if(NAS_MMC_NV_ITEM_ACTIVE == stSgsnrVersion.ucStatus)
    {
        pstMs3GppRel->enMsSgsnRel = stSgsnrVersion.ucUeSgsnrVersion;
    }

    return;
}
VOS_VOID NAS_MMC_ReadClassMark1Nvim(VOS_VOID)
{
    VOS_UINT8                           ucClassmark1;
    NAS_MML_MS_CAPACILITY_INFO_STRU    *pstMsCapability = VOS_NULL_PTR;


    ucClassmark1 = NAS_MML_DEFAULT_CLASSMARK1_VALUE;

    pstMsCapability = NAS_MML_GetMsCapability();

    if (NV_OK != NV_Read (en_NV_Item_Gsm_Classmark1,
                          &ucClassmark1, sizeof(VOS_UINT8)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,  "NAS_MMC_ReadClassMark1Nvim(): NV_Read Classmark1 Error");
        return;
    }

    pstMsCapability->ucClassmark1 = ucClassmark1;
    return;
}


VOS_VOID NAS_MMC_ReadClassMark2Nvim(VOS_VOID)
{
    VOS_UINT8                           aucClassmark2[NAS_MML_CLASSMARK2_LEN];
    NAS_MML_MS_CAPACILITY_INFO_STRU    *pstMsCapability = VOS_NULL_PTR;


    PS_MEM_SET(aucClassmark2, 0x00, NAS_MML_CLASSMARK2_LEN);

    pstMsCapability = NAS_MML_GetMsCapability();

    if (NV_OK != NV_Read (en_NV_Item_Gsm_Classmark2,
                          aucClassmark2, NAS_MML_CLASSMARK2_LEN))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadClassMark2Nvim():NV_Read Classmark2 Error");
        return;
    }

    PS_MEM_CPY(pstMsCapability->aucClassmark2, aucClassmark2, NAS_MML_CLASSMARK2_LEN);

    return;
}


VOS_VOID NAS_MMC_UpdateNetworkCapabilityGeaValue(VOS_VOID)
{
    NAS_MMC_NVIM_GPRS_GEA_ALG_CTRL_STRU stGeaSupportCtrl;
    VOS_UINT8                           ucGeaSupportCtrl;
    NAS_MML_MS_CAPACILITY_INFO_STRU    *pstMsCapability = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength = 0;

    stGeaSupportCtrl.ucStatus         = NAS_MMC_NV_ITEM_DEACTIVE;
    stGeaSupportCtrl.ucGeaSupportCtrl = 0;

    pstMsCapability = NAS_MML_GetMsCapability();

    NV_GetLength(en_NV_Item_GEA_SUPPORT_CTRL, &ulLength);
    if (ulLength > sizeof(stGeaSupportCtrl))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_GEA_SUPPORT_CTRL, &stGeaSupportCtrl, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_UpdateNetworkCapabilityGeaValue():NV_Read en_NV_Item_GEA_SUPPORT_CTRL Error");

        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stGeaSupportCtrl.ucStatus)
    {
        ucGeaSupportCtrl = stGeaSupportCtrl.ucGeaSupportCtrl;

        /* 初始化为GEA1-GEA7都不支持 */
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0] &= ~NAS_MMC_GPRS_GEA1_VALUE;
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] &= ~( NAS_MMC_GPRS_GEA2_VALUE
                                                                          | NAS_MMC_GPRS_GEA3_VALUE
                                                                          | NAS_MMC_GPRS_GEA4_VALUE
                                                                          | NAS_MMC_GPRS_GEA5_VALUE
                                                                          | NAS_MMC_GPRS_GEA6_VALUE
                                                                          | NAS_MMC_GPRS_GEA7_VALUE);

        /*  GEA1: 第一个字节的最高位 */
        if (NAS_MMC_GPRS_GEA1_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA1_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0] |= NAS_MMC_GPRS_GEA1_VALUE;
        }

        /*  GEA2: 第二个字节的左起第二个bit */
        if (NAS_MMC_GPRS_GEA2_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA2_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] |= NAS_MMC_GPRS_GEA2_VALUE;
        }

        /* GEA3: 第二个字节的左起第三个bit */
        if (NAS_MMC_GPRS_GEA3_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA3_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] |= NAS_MMC_GPRS_GEA3_VALUE;
        }

        /* GEA4: 第二个字节的左起第四个bit */
        if (NAS_MMC_GPRS_GEA4_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA4_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] |= NAS_MMC_GPRS_GEA4_VALUE;
        }

        /* GEA5: 第二个字节的左起第五个bit */
        if (NAS_MMC_GPRS_GEA5_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA5_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] |= NAS_MMC_GPRS_GEA5_VALUE;
        }

        /* GEA6: 第二个字节的左起第六个bit */
        if (NAS_MMC_GPRS_GEA6_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA6_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] |= NAS_MMC_GPRS_GEA6_VALUE;
        }

        /* GEA7: 第二个字节的左起第七个bit */
        if (NAS_MMC_GPRS_GEA7_SUPPORT == (ucGeaSupportCtrl & NAS_MMC_GPRS_GEA7_SUPPORT))
        {
            pstMsCapability->stMsNetworkCapability.aucNetworkCapability[1] |= NAS_MMC_GPRS_GEA7_VALUE;
        }
    }

    return;
}



VOS_VOID NAS_MMC_ReadNetworkCapabilityNvim(VOS_VOID)
{
    NAS_MMC_NVIM_NETWORK_CAPABILITY_STRU                    stNetworkCap;/*network capability*/

    VOS_UINT32                          ulLen;
    NAS_MML_MS_CAPACILITY_INFO_STRU    *pstMsCapability = VOS_NULL_PTR;


    ulLen = 0;
    PS_MEM_SET(&stNetworkCap, 0x00, sizeof(NAS_MMC_NVIM_NETWORK_CAPABILITY_STRU));

    pstMsCapability = NAS_MML_GetMsCapability();
    NV_GetLength(en_NV_Item_NetworkCapability, &ulLen);

    if (ulLen > (NAS_MML_MAX_NETWORK_CAPABILITY_LEN + 1))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadNetworkCapabilityNvim():WARNING: en_NV_Item_NetworkCapability length Error");
        return;
    }
    else
    {
        if(NV_OK != NV_Read (en_NV_Item_NetworkCapability, &stNetworkCap, ulLen))
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadNetworkCapabilityNvim():en_NV_Item_NetworkCapability Error");
            return;
        }
    }

    /* nv中aucNetworkCapability[0]存的长度为networkCapbility长度 + networkCapbility值*/
    pstMsCapability->stMsNetworkCapability.ucNetworkCapabilityLen = stNetworkCap.aucNetworkCapability[0];

    if (pstMsCapability->stMsNetworkCapability.ucNetworkCapabilityLen > NAS_MML_MAX_NETWORK_CAPABILITY_LEN)
    {
        pstMsCapability->stMsNetworkCapability.ucNetworkCapabilityLen = NAS_MML_MAX_NETWORK_CAPABILITY_LEN;
    }

    /* 存贮网络能力 */
    PS_MEM_CPY(&pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0],
               &(stNetworkCap.aucNetworkCapability[1]),
               pstMsCapability->stMsNetworkCapability.ucNetworkCapabilityLen);

    /* 根据en_NV_Item_GEA_SUPPORT_CTRL更新network capability的GEA的值 */
    NAS_MMC_UpdateNetworkCapabilityGeaValue();
    return;
}


VOS_VOID NAS_MMC_ReadUcs2CustomizationNvim(VOS_VOID)
{
    NAS_MML_MS_CAPACILITY_INFO_STRU                        *pstMsCapability = VOS_NULL_PTR;
    VOS_UINT32                                              ulDataLen;
    VOS_UINT32                                              ulResult;
    NAS_MMC_NVIM_UCS2_CUSTOMIZATION_STRU                    stUcs2Customization;

    ulDataLen       = 0;
    pstMsCapability = NAS_MML_GetMsCapability();

    stUcs2Customization.usUcs2Customization = NAS_MMC_UCS2_NO_PREFER;

    NV_GetLength(en_NV_Item_UCS2_Customization, &ulDataLen);
    if (ulDataLen > sizeof(stUcs2Customization))
    {
        return;
    }

    ulResult = NV_Read(en_NV_Item_UCS2_Customization, &stUcs2Customization, ulDataLen);

    /* 读取失败，默认 the ME has no preference between the use of the default alphabet and the use of UCS2 */
    if ( NV_OK != ulResult )
    {
        stUcs2Customization.usUcs2Customization = NAS_MMC_UCS2_NO_PREFER;
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadUcs2CustomizationNvim():en_NV_Item_UCS2_Customization Error");
    }

    /* 更新classmark2中的UCS2能力,更新ms network capability中的UCS2的能力 */
    if (NAS_MMC_UCS2_NO_PREFER == stUcs2Customization.usUcs2Customization)
    {
        pstMsCapability->aucClassmark2[3] |= 0x10;
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0] |= 0x10;
    }
    else
    {
        pstMsCapability->aucClassmark2[3] &= 0xef;
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0] &= 0xef;
    }

    return;
}



VOS_VOID NAS_MMC_ReadCloseSmsCapabilityConfigNvim(VOS_VOID)
{
    NAS_MML_MS_CAPACILITY_INFO_STRU                        *pstMsCapability = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;
    NAS_NVIM_CLOSE_SMS_CAPABILITY_CFG_STRU                  stCloseSmsCapabilityCfg;
    VOS_UINT32                                              ulRet;

    ulLength        = 0;

    pstMsCapability = NAS_MML_GetMsCapability();

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Close_SMS_Capability_Config, &ulLength);

    if (ulLength > sizeof(NAS_NVIM_CLOSE_SMS_CAPABILITY_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCloseSmsCapabilityConfigNvim():en_NV_Item_Close_SMS_Capability_Config length Error");
        return;
    }

    ulRet = NV_Read(en_NV_Item_Close_SMS_Capability_Config,
                   &stCloseSmsCapabilityCfg,
                    ulLength);

    /* 读取关闭短信定制NV失败,默认不关闭短信功能 */
    if (NV_OK != ulRet)
    {
        stCloseSmsCapabilityCfg.ucActFlg = VOS_FALSE;

        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCloseSmsCapabilityConfigNvim():en_NV_Item_CLOSE_SMS_Capability_Config Failed");
    }

    /* 更新ms network capability中的CS/PS的短信支持能力,更新class mark2中的PS短信能力 */
    if (VOS_TRUE == stCloseSmsCapabilityCfg.ucActFlg)
    {
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0] &= 0x9f;
        pstMsCapability->aucClassmark2[2] &= 0xf7;
    }
    else
    {
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[0] |= 0x60;
        pstMsCapability->aucClassmark2[2] |= 0x08;
    }

    return;
}


#if (FEATURE_ON == FEATURE_LTE)

VOS_VOID NAS_MMC_ReadLteRoamConfigNvim(VOS_VOID)
{
    NAS_MMC_NVIM_LTE_INTERNATIONAL_ROAM_CFG_STRU            stLteRoamNvCfg;
    VOS_UINT32                                              ulLength;
    VOS_UINT32                                              aulRoamEnabledMccList[5];


    ulLength = 0;

    PS_MEM_SET(&stLteRoamNvCfg,       0x00, sizeof(NAS_MMC_NVIM_LTE_INTERNATIONAL_ROAM_CFG_STRU));
    PS_MEM_SET(aulRoamEnabledMccList, 0x00, sizeof(aulRoamEnabledMccList));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Lte_Internation_Roam_Config, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_LTE_INTERNATIONAL_ROAM_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteRoamConfigNvim():WARNING: en_NV_Item_Lte_Internation_Roam_Config length Error");
        return;
    }

    if ( NV_OK != NV_Read(en_NV_Item_Lte_Internation_Roam_Config,
                          &stLteRoamNvCfg,
                           ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteRoamConfigNvim(): en_NV_Item_Lte_Internation_Roam_Config Error");
        return;
    }

    PS_MEM_CPY(aulRoamEnabledMccList, stLteRoamNvCfg.aucRoamEnabledMccList, sizeof(aulRoamEnabledMccList));

    /* 设置当前LTE国际漫游是否允许 */
    NAS_MML_SetLteRoamAllowedFlg(stLteRoamNvCfg.ucLteRoamAllowedFlg);

    /* 设置允许LTE漫游的国家码 */
    NAS_MML_SetRoamEnabledMccList(aulRoamEnabledMccList);

    return;
}


VOS_VOID NAS_MMC_ReadLNasReleaseNvim(VOS_VOID)
{
    LNAS_LMM_NV_NAS_RELEASE_STRU        stNvNasRelease;
    VOS_UINT32                          ulLength;
    NAS_MML_MS_3GPP_REL_STRU           *pstMs3GppRel = VOS_NULL_PTR;

    pstMs3GppRel = NAS_MML_GetMs3GppRel();

    pstMs3GppRel->enLteNasRelease = NAS_MML_3GPP_REL_R9;

    /* 获取NV EN_NV_ID_NAS_RELEASE 长度 */
    NV_GetLength(EN_NV_ID_NAS_RELEASE, &ulLength);

    if (ulLength > sizeof(LNAS_LMM_NV_NAS_RELEASE_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLNasReleaseNvim():WARNING: EN_NV_ID_NAS_RELEASE length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(EN_NV_ID_NAS_RELEASE,
                         &stNvNasRelease, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadLNasReleaseNvim(): EN_NV_ID_NAS_RELEASE Error");
        return;
    }

    if (VOS_TRUE == stNvNasRelease.bitOpReleaseName)
    {
        if (0x00000a00 <= stNvNasRelease.ulReleaseName)
        {
            pstMs3GppRel->enLteNasRelease = NAS_MML_3GPP_REL_R10;
        }
    }

    return;
}

#endif


VOS_VOID NAS_MMC_ReadRPLMNConfigNvim(VOS_VOID)
{
    NAS_MMC_NVIM_ROAM_SEARCH_RPLMN_CFG_STRU            stRoamPlmnSearchNvCfg;
    VOS_UINT32                                         ulLength = 0;

    PS_MEM_SET(&stRoamPlmnSearchNvCfg, 0x00, sizeof(NAS_MMC_NVIM_ROAM_SEARCH_RPLMN_CFG_STRU));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Roam_Search_Rplmn_Config, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_ROAM_SEARCH_RPLMN_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteRoamConfigNvim():WARNING: en_NV_Item_Roam_Search_Rplmn_Config length Error");
        return;
    }

    if ( NV_OK != NV_Read(en_NV_Item_Roam_Search_Rplmn_Config,
                          &stRoamPlmnSearchNvCfg,
                           ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteRoamConfigNvim(): en_NV_Item_Roam_Search_Rplmn_Config Error");
        return;
    }

    /*保存漫游先搜RPLMN标识*/
    NAS_MML_SetRoamRplmnSearchFlg(stRoamPlmnSearchNvCfg.ucRoamRplmnflg);
    /*保存国内Mcc列表*/
    NAS_MML_SetNationalMCCList(stRoamPlmnSearchNvCfg.aucRoamEnabledMccList);

    return;
}
VOS_VOID NAS_MMC_ReadSBMCustomDualIMSIConfigNvim(VOS_VOID)
{
    NVIM_SBM_CUSTOM_DUAL_IMSI_STRU             stSBMDualimsiNvCfg;
    VOS_UINT32                                         ulLength;
    ulLength = 0;

    PS_MEM_SET(&stSBMDualimsiNvCfg, 0x00, sizeof(NVIM_SBM_CUSTOM_DUAL_IMSI_STRU));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_SBM_CUSTOM_DUAL_IMSI, &ulLength);

    if (ulLength > sizeof(NVIM_SBM_CUSTOM_DUAL_IMSI_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteRoamConfigNvim():WARNING: en_NV_Item_SBM_CUSTOM_DUAL_IMSI length Error");
        return;
    }

    if ( NV_OK != NV_Read(en_NV_Item_SBM_CUSTOM_DUAL_IMSI,
                          &stSBMDualimsiNvCfg,
                           ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteRoamConfigNvim(): en_NV_Item_SBM_CUSTOM_DUAL_IMSI Error");
        return;
    }

    /*保存软银定制开关*/
    NAS_MML_SetDisableLTEOnBandFlg(stSBMDualimsiNvCfg.ucSbmCustomDualImsiFlg);

    return;
}




VOS_VOID NAS_MMC_ReadDailRejectConfigNvim(VOS_VOID)
{
    NAS_MMC_NVIM_DAIL_REJECT_CFG_STRU   stDellRejectCfg;
    VOS_UINT32                          ulLength;


    stDellRejectCfg.ucEnableFlg = VOS_FALSE;


    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Dail_Reject_Config, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_DAIL_REJECT_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDailRejectConfigNvim():WARNING: en_NV_Item_Dail_Reject_Config length Error");
        return;
    }

    if ( NV_OK != NV_Read(en_NV_Item_Dail_Reject_Config,
                          &stDellRejectCfg,
                          ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDellRejectConfigNvim(): en_NV_Item_Dell_Reject_Config Error");
        return;
    }

    /* 设置当前拨号被拒是否可用 */
    NAS_MML_SetDailRejectCfg(stDellRejectCfg.ucEnableFlg);

    return;
}



VOS_VOID  NAS_MMC_ReadMsModeNvim( VOS_VOID )
{
    NAS_NVIM_MS_CLASS_STRU              stMsClass;
    VOS_UINT32                          ulLength;

    ulLength        = 0;

    PS_MEM_SET(&stMsClass, 0x00, sizeof(NAS_NVIM_MS_CLASS_STRU));
    stMsClass.ucMsClass = NAS_MML_MS_MODE_BUTT;

    NV_GetLength(en_NV_Item_MMA_MsClass, &ulLength);
    if (ulLength > sizeof(NAS_NVIM_MS_CLASS_STRU))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_MMA_MsClass, &stMsClass, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadMsModeNvim():en_NV_Item_MMA_MsClass NV Read Fail!");
        return;
    }

    /* NV中的取值定义( 0：只支持CS域手机；1：NV_MS_CLASS_CG，只支持PS域手机；
       2：NV_MS_CLASS_A，支持CS/PS域手机（默认值）)与MML中的不一致，需要转换*/

    if ((NV_MS_MODE_CS_ONLY == stMsClass.ucMsClass)
    ||  (NV_MS_MODE_ANY == stMsClass.ucMsClass))
    {
        NAS_MML_SetMsMode(NAS_MML_MS_MODE_CS_ONLY);
    }

    else if (NV_MS_MODE_PS_ONLY == stMsClass.ucMsClass)
    {
        NAS_MML_SetMsMode(NAS_MML_MS_MODE_PS_ONLY);
    }
    else if (NV_MS_MODE_CS_PS == stMsClass.ucMsClass)
    {
        NAS_MML_SetMsMode(NAS_MML_MS_MODE_PS_CS);
    }
    else
    {
    }

    return;
}
VOS_VOID NAS_MMC_ReadUserSetBandNvim(VOS_VOID)
{
    NVIM_UE_SUPPORT_FREQ_BAND_STRU                          stUserSetFreqBand;
    NAS_MML_MS_BAND_INFO_STRU                              *pstUserSetBand = VOS_NULL_PTR;
    LTE_BAND_STRU                                           stLteBand;


    PS_MEM_SET(&stUserSetFreqBand, 0x00, sizeof(stUserSetFreqBand));
    PS_MEM_SET(&stLteBand,         0x00, sizeof(stLteBand));

    pstUserSetBand = NAS_MML_GetMsSupportBand();

    /* 读用户设置的GU频段 NV项 */
    if (NV_OK != NV_Read(en_NV_Item_Support_Freqbands,
                         &stUserSetFreqBand, sizeof(NVIM_UE_SUPPORT_FREQ_BAND_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadUserSetBandNvim():en_NV_Item_Support_Freqbands Error");
        return;
    }

    pstUserSetBand->unGsmBand.ulBand   = stUserSetFreqBand.ulGsmBand;
    pstUserSetBand->unWcdmaBand.ulBand = stUserSetFreqBand.ulWcdmaBand;

    /* 读用户设置的LTE频段 NV项 */
    if (NV_OK != NV_Read(en_NV_Item_USER_SET_LTEBANDS,
                         &stLteBand, sizeof(LTE_BAND_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadUserSetBandNvim():en_NV_Item_USER_SET_LTEBANDS Error");
        return;
    }

    pstUserSetBand->stLteBand = stLteBand;

    return;
}


VOS_VOID NAS_MMC_ReadUseSingleRplmnFlagNvim(VOS_VOID)
{
    NAS_MMC_NVIM_USE_SINGLE_RPLMN_STRU  stUseSingleRplmnFlag;
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength        = 0;


    stUseSingleRplmnFlag.usUseSingleRplmnFlag = NAS_MMC_NV_ITEM_ACTIVE;

    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    NV_GetLength(en_NV_Item_Use_Single_Rplmn_When_Area_Lost, &ulLength);
    if (ulLength > sizeof(stUseSingleRplmnFlag))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_Use_Single_Rplmn_When_Area_Lost,
                         &stUseSingleRplmnFlag, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadUseSingleRplmnFlagNvim():en_NV_Item_Use_Single_Rplmn_When_Area_Lost Error");

        return;
    }

    if ((stUseSingleRplmnFlag.usUseSingleRplmnFlag != NAS_MMC_NV_ITEM_DEACTIVE)
     && (stUseSingleRplmnFlag.usUseSingleRplmnFlag != NAS_MMC_NV_ITEM_ACTIVE))
    {
        return;
    }

    /* usUseSingleRplmnFlag为0表WCDMA和GSM存储各自注册的RPLMN */
    if (NAS_MMC_NV_ITEM_DEACTIVE == stUseSingleRplmnFlag.usUseSingleRplmnFlag)
    {
        /* 支持双RPLMN */
        pstRplmnCfgInfo->ucMultiRATRplmnFlg = VOS_TRUE;
    }
    else
    {
        pstRplmnCfgInfo->ucMultiRATRplmnFlg = VOS_FALSE;
    }


    return;
}


VOS_VOID NAS_MMC_ReadRplmnWithRatNvim(VOS_VOID)
{
    NAS_NVIM_RPLMN_WITH_RAT_STRU        stRplmn;
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;
    NAS_MML_PLMN_RAT_PRIO_STRU         *pstRatPrioList = VOS_NULL_PTR;

    pstRatPrioList           = NAS_MML_GetMsPrioRatList();


    stRplmn.stGRplmn.ulMcc          = NAS_MML_INVALID_MCC;
    stRplmn.stGRplmn.ulMnc          = NAS_MML_INVALID_MNC;
    stRplmn.stWRplmn.ulMcc          = NAS_MML_INVALID_MCC;
    stRplmn.stWRplmn.ulMnc          = NAS_MML_INVALID_MNC;
    stRplmn.ucLastRplmnRat          = NAS_MML_NET_RAT_TYPE_BUTT;
    stRplmn.ucLastRplmnRatEnableFlg = VOS_FALSE;

    pstRplmnCfgInfo   = NAS_MML_GetRplmnCfg();

    if(NV_OK != NV_Read (en_NV_Item_RPlmnWithRat, &stRplmn, sizeof(NAS_NVIM_RPLMN_WITH_RAT_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRplmnWithRatNvim(): en_NV_Item_RPlmnWithRat Error");
        return;
    }

    if ((stRplmn.ucLastRplmnRatEnableFlg != NAS_MMC_NV_ITEM_DEACTIVE)
     && (stRplmn.ucLastRplmnRatEnableFlg != NAS_MMC_NV_ITEM_ACTIVE))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRplmnWithRatNvim(): NV parameter Error");
        return;
    }

    pstRplmnCfgInfo->ucLastRplmnRatFlg = stRplmn.ucLastRplmnRatEnableFlg;

    /* 单板首次升级此值为默认的0XFF,此时需要按照默认的SYSCFGEX的接入优先级取
       最高优先级 */
    if ((NAS_MML_NET_RAT_TYPE_INVALID == stRplmn.ucLastRplmnRat)
     || (NAS_MML_NET_RAT_TYPE_BUTT    == stRplmn.ucLastRplmnRat))
    {
        pstRplmnCfgInfo->enLastRplmnRat = pstRatPrioList->aucRatPrio[0];
    }
    else
    {
        pstRplmnCfgInfo->enLastRplmnRat    = stRplmn.ucLastRplmnRat;
    }

    PS_MEM_CPY(&pstRplmnCfgInfo->stGRplmnInNV, &stRplmn.stGRplmn, sizeof(NAS_MML_PLMN_ID_STRU));
    PS_MEM_CPY(&pstRplmnCfgInfo->stWRplmnInNV, &stRplmn.stWRplmn, sizeof(NAS_MML_PLMN_ID_STRU));


    return;
}



VOS_VOID NAS_MMC_ReadNvimLastRplmnRat(
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8    *penNvimLastRplmnRat
)
{
    NAS_NVIM_RPLMN_WITH_RAT_STRU        stRplmn;    

    stRplmn.stGRplmn.ulMcc          = NAS_MML_INVALID_MCC;
    stRplmn.stGRplmn.ulMnc          = NAS_MML_INVALID_MNC;
    stRplmn.stWRplmn.ulMcc          = NAS_MML_INVALID_MCC;
    stRplmn.stWRplmn.ulMnc          = NAS_MML_INVALID_MNC;
    stRplmn.ucLastRplmnRat          = NAS_MML_NET_RAT_TYPE_BUTT;
    stRplmn.ucLastRplmnRatEnableFlg = VOS_FALSE;

    if(NV_OK != NV_Read (en_NV_Item_RPlmnWithRat, &stRplmn, sizeof(NAS_NVIM_RPLMN_WITH_RAT_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadNvimLastRplmnRat(): en_NV_Item_RPlmnWithRat Error");
        return;
    }

    if ((stRplmn.ucLastRplmnRatEnableFlg != NAS_MMC_NV_ITEM_DEACTIVE)
     && (stRplmn.ucLastRplmnRatEnableFlg != NAS_MMC_NV_ITEM_ACTIVE))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadNvimLastRplmnRat(): NV parameter Error");
        return;
    }

    *penNvimLastRplmnRat = stRplmn.ucLastRplmnRat;
    
    return;
}




VOS_VOID NAS_MMC_WriteRplmnWithRatNvim(VOS_VOID)
{
    NAS_NVIM_RPLMN_WITH_RAT_STRU        stRplmn;
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;

    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    stRplmn.ucLastRplmnRat          = pstRplmnCfgInfo->enLastRplmnRat;
    stRplmn.stGRplmn.ulMcc          = pstRplmnCfgInfo->stGRplmnInNV.ulMcc;
    stRplmn.stGRplmn.ulMnc          = pstRplmnCfgInfo->stGRplmnInNV.ulMnc;
    stRplmn.stWRplmn.ulMcc          = pstRplmnCfgInfo->stWRplmnInNV.ulMcc;
    stRplmn.stWRplmn.ulMnc          = pstRplmnCfgInfo->stWRplmnInNV.ulMnc;
    stRplmn.ucLastRplmnRatEnableFlg = pstRplmnCfgInfo->ucLastRplmnRatFlg;

    if(NV_OK != NV_Write(en_NV_Item_RPlmnWithRat, &stRplmn, sizeof(NAS_NVIM_RPLMN_WITH_RAT_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_WriteRplmnWithRatNvim(): en_NV_Item_RPlmnWithRat Error");
    }

    return;
}


#if   (FEATURE_ON == FEATURE_LTE)


VOS_VOID NAS_MMC_ReadTinInfoNvim( VOS_VOID )
{
    NAS_NVIM_TIN_INFO_STRU          stTinInfo;

    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;


    PS_MEM_SET(&stTinInfo, 0x00, sizeof(stTinInfo));

    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    if ( NV_OK != NV_Read(en_NV_Item_TIN_INFO,
                          &stTinInfo,
                          sizeof(stTinInfo) ))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadTinInfoNvim(): en_NV_Item_TIN_INFO Error");

        return;
    }

    PS_MEM_CPY(pstRplmnCfgInfo->aucLastImsi, stTinInfo.aucImsi, sizeof(pstRplmnCfgInfo->aucLastImsi));
    pstRplmnCfgInfo->enTinType = (NAS_MML_TIN_TYPE_ENUM_UINT8)stTinInfo.ucTinType;

    /* 如果平台不支持LTE，需要对Tin type进行异常保护 */
    if (VOS_FALSE == NAS_MML_IsPlatformSupportLte())
    {
        if ((NAS_MML_TIN_TYPE_GUTI == pstRplmnCfgInfo->enTinType)
         || (NAS_MML_TIN_TYPE_RAT_RELATED_TMSI == pstRplmnCfgInfo->enTinType))
        {
            pstRplmnCfgInfo->enTinType = NAS_MML_TIN_TYPE_INVALID;
        }
    }

    return;

}
VOS_VOID  NAS_MMC_WriteTinInfoNvim(
    NAS_MML_TIN_TYPE_ENUM_UINT8         enTinType,
    VOS_UINT8                          *pucImsi
)
{
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;

    NAS_NVIM_TIN_INFO_STRU              stTinInfo;

    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    /*
        The following EMM parameter shall be stored in a non-volatile memory in the
        ME together with the IMSI from the USIM:
        -   TIN.
        This EMM parameter can only be used if the IMSI from the USIM matches the
        IMSI stored in the non-volatile memory of the ME; else the UE shall delete
        the EMM parameter.
    */

    /* 先更新MML的参数 */
    pstRplmnCfgInfo->enTinType  = enTinType;
    PS_MEM_CPY(pstRplmnCfgInfo->aucLastImsi, pucImsi, sizeof(pstRplmnCfgInfo->aucLastImsi));

    /* 再更新NVIM中的参数 */
    stTinInfo.ucTinType        = pstRplmnCfgInfo->enTinType;
    PS_MEM_CPY(stTinInfo.aucImsi, pstRplmnCfgInfo->aucLastImsi, sizeof(stTinInfo.aucImsi));
    PS_MEM_SET(stTinInfo.aucReserve, 0, sizeof(stTinInfo.aucReserve));

    /* 保存在NVIM中 */
    if (NV_OK != NV_Write (en_NV_Item_TIN_INFO,
                           &stTinInfo,
                           sizeof(stTinInfo)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_WriteTinInfoNvim:Write NV Failed");
    }
}


VOS_VOID NAS_MMC_ReadLteCsServiceConfigNvim(VOS_VOID)
{
    NAS_NVIM_LTE_CS_SERVICE_CFG_STRU                        stLteCsServiceCfg;
    VOS_UINT32                                              ulLength;

    ulLength        = 0;


    PS_MEM_SET(&stLteCsServiceCfg, 0x00, sizeof(NAS_NVIM_LTE_CS_SERVICE_CFG_STRU));
    NV_GetLength(en_NV_Item_Lte_Cs_Service_Config, &ulLength);
    if (ulLength > sizeof(stLteCsServiceCfg))
    {
        return;
    }
    if ( NV_OK != NV_Read(en_NV_Item_Lte_Cs_Service_Config,
                          &stLteCsServiceCfg,
                          ulLength ))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteCsServiceConfigNvim(): en_NV_Item_Lte_Cs_Service_Config Error");
        return;
    }

    /* 如果NV项激活并且值有效，则更新MML中的全局变量 */
    if ((NAS_MMC_NV_ITEM_ACTIVE == stLteCsServiceCfg.ucNvimActiveFlg)
     && (stLteCsServiceCfg.ucLteCsServiceCfg < NAS_MML_LTE_SUPPORT_BUTT))
    {
        /* 设置当前LTE的CS业务配置能力的NV项激活 */
        NAS_MML_SetLteCsServiceCfg(stLteCsServiceCfg.ucLteCsServiceCfg);

#if (VOS_WIN32 == VOS_OS_VER)
        NAS_MML_SetLteCsServiceCfg(NAS_MML_LTE_SUPPORT_CSFB_AND_SMS_OVER_SGS);
#endif

        return;
    }

    /* NV无效时设置为  */
    NAS_MML_SetLteCsServiceCfg(NAS_MML_LTE_SUPPORT_BUTT);

#if (VOS_WIN32 == VOS_OS_VER)
        NAS_MML_SetLteCsServiceCfg(NAS_MML_LTE_SUPPORT_CSFB_AND_SMS_OVER_SGS);
#endif


    return;
}





VOS_VOID NAS_MMC_ReadHoWaitSysinfoTimeLenNvim(VOS_VOID)
{
    NAS_MMC_NVIM_HO_WAIT_SYSINFO_TIMER_CFG_STRU             stHoTimerCfg;


    PS_MEM_SET(&stHoTimerCfg, 0x00, sizeof(stHoTimerCfg));

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_Ho_Wait_Sysinfo_Timer_Config,
                         &stHoTimerCfg, sizeof(NAS_MMC_NVIM_HO_WAIT_SYSINFO_TIMER_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadHoWaitSysinfoTimeLenNvim(): en_NV_Item_HO_WAIT_SYSINFO_TIMER_Config Error");
        return;
    }

    /* en_NV_Item_HO_WAIT_SYSINFO_TIMER_Config未使能，则设置等系统消息时间为默认值1s */
    if ( NAS_MMC_NV_ITEM_DEACTIVE == stHoTimerCfg.ucNvimActiveFlg )
    {
        NAS_MML_SetHoWaitSysinfoTimerLen(NAS_MML_HO_WAIT_SYS_INFO_DEFAULT_LEN_VALUE);
        return;
    }

    /* en_NV_Item_HO_WAIT_SYSINFO_TIMER_Config设置等系统消息时间为0，则更新为默认值1s */
    if ( 0 == stHoTimerCfg.ucWaitSysinfoTimeLen )
    {
        NAS_MML_SetHoWaitSysinfoTimerLen(NAS_MML_HO_WAIT_SYS_INFO_DEFAULT_LEN_VALUE);
        return;
    }

    /* 更新MML全局变量中的HO等系统消息的值 */
    NAS_MML_SetHoWaitSysinfoTimerLen(stHoTimerCfg.ucWaitSysinfoTimeLen * NAS_MML_ONE_THOUSAND_MILLISECOND);

    return;
}
#endif

VOS_VOID NAS_MMC_ReadCsfbEmgCallLaiChgLauFirstNvim(VOS_VOID)
{
    NAS_MMC_NVIM_CSFB_EMG_CALL_LAI_CHG_LAU_FIRST_CFG_STRU   stCsfbEmgCallLauFirstCfg;
    VOS_UINT32                                              ulLength;


    PS_MEM_SET(&stCsfbEmgCallLauFirstCfg, 0x00, sizeof(NAS_MMC_NVIM_CSFB_EMG_CALL_LAI_CHG_LAU_FIRST_CFG_STRU));

    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Csfb_Emg_Call_LaiChg_Lau_First_CFG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_CSFB_EMG_CALL_LAI_CHG_LAU_FIRST_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCsfbEmgCallLaiChgLauFirstNvim():WARNING: en_NV_Item_Csfb_Emergency_Call_LaiChg_Lau_First_CFG length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_Csfb_Emg_Call_LaiChg_Lau_First_CFG,
                         &stCsfbEmgCallLauFirstCfg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadCsfbEmgCallLaiChgLauFirstNvim(): en_NV_Item_Csfb_Emergency_Call_LaiChg_Lau_First_CFG Error");
        return;
    }

    if (VOS_TRUE == stCsfbEmgCallLauFirstCfg.ucCsfbEmgCallLaiChgLauFirstFlg)
    {
        NAS_MML_SetCsfbEmgCallLaiChgLauFirstFlg(stCsfbEmgCallLauFirstCfg.ucCsfbEmgCallLaiChgLauFirstFlg);
    }

    return;
}





VOS_VOID NAS_MMC_ReadEHPlmnSupportNvim(VOS_VOID)
{
    NAS_MMC_NVIM_EHPLMN_SUPPORT_FLG_STRU                    stEHPlmnSupportFlg;
    NAS_MML_HPLMN_CFG_INFO_STRU                            *pstHplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength        = 0;

    stEHPlmnSupportFlg.usEHPlmnSupportFlg = VOS_FALSE;

    pstHplmnCfgInfo = NAS_MML_GetHplmnCfg();

    NV_GetLength(en_NV_Item_Standard_EHplmn_Support_Flg, &ulLength);
    if (ulLength > sizeof(stEHPlmnSupportFlg))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_Standard_EHplmn_Support_Flg,
                         &stEHPlmnSupportFlg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadEHPlmnSupportNvim(): en_NV_Item_Standard_EHplmn_Support_Flg Error");
        return;
    }

    if ((NAS_MMC_NV_ITEM_ACTIVE != stEHPlmnSupportFlg.usEHPlmnSupportFlg)
     && (NAS_MMC_NV_ITEM_DEACTIVE != stEHPlmnSupportFlg.usEHPlmnSupportFlg))
    {
        return;
    }

    pstHplmnCfgInfo->ucEhplmnSupportFlg = (VOS_UINT8)stEHPlmnSupportFlg.usEHPlmnSupportFlg;

    return;
}


VOS_VOID NAS_MMC_ReadActingHPlmnSupportNvim(VOS_VOID)
{
    NVIM_ACTING_HPLMN_SUPPORT_FLAG_STRU                     stActingHPlmnSupport;
    VOS_UINT32                                              ulLength;


    PS_MEM_SET(&stActingHPlmnSupport, 0x00, sizeof(NVIM_ACTING_HPLMN_SUPPORT_FLAG_STRU));
    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_ACTING_PLMN_SUPPORT_FLAG, &ulLength);

    if (ulLength > sizeof(NVIM_ACTING_HPLMN_SUPPORT_FLAG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadActingHPlmnSupportNvim():WARNING: en_NV_Item_ACTING_PLMN_SUPPORT_FLAG length Error");
        return;
    }

    /* 读取ACTING-HPLMN控制NV */
    if (NV_OK != NV_Read(en_NV_Item_ACTING_PLMN_SUPPORT_FLAG,
                         &stActingHPlmnSupport, sizeof(NVIM_ACTING_HPLMN_SUPPORT_FLAG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadActingHPlmnSupportNvim(): en_NV_Item_ACTING_PLMN_SUPPORT_FLAG Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stActingHPlmnSupport.ucNvimActiveFlg)
    {
        /* 赋值ACTING-HPLMN控制标志 */
        NAS_MML_SetActingHplmnSupportFlg(VOS_TRUE);
    }

    return;
}



VOS_VOID NAS_MMC_ReadSearchHPlmnFlgAutoModeNvim(VOS_VOID)
{
    NAS_MMC_NVIM_AUTO_SEARCH_HPLMN_FLG_STRU                 stHplmnSearchFlg;
    NAS_MML_HPLMN_CFG_INFO_STRU                            *pstHplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength        = 0;

    stHplmnSearchFlg.usAutoSearchHplmnFlg  = 0;

    pstHplmnCfgInfo = NAS_MML_GetHplmnCfg();

    NV_GetLength(en_NV_Item_SrchHplmnFlg_StartOrAreaLostInAutoMode, &ulLength);
    if (ulLength > sizeof(stHplmnSearchFlg))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_SrchHplmnFlg_StartOrAreaLostInAutoMode,
                         &stHplmnSearchFlg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadSearchHPlmnFlgAutoModeNvim():en_NV_Item_SrchHplmnFlg_StartOrAreaLostInAutoMode Error");
        return;
    }

    if (stHplmnSearchFlg.usAutoSearchHplmnFlg >= NAS_MML_SEARCH_HPLMN_FLG_TYPE_BUTT )
    {
        return;
    }

    pstHplmnCfgInfo->enAutoSearchHplmnFlg = (VOS_UINT8)stHplmnSearchFlg.usAutoSearchHplmnFlg;
    return;
}



VOS_VOID NAS_MMC_ReadSearchHPlmnFlgManualModeNvim(VOS_VOID)
{
    NAS_MMC_NVIM_MANUAL_SEARCH_HPLMN_FLG_STRU               stManualSearchHplmnFlg;
    NAS_MML_HPLMN_CFG_INFO_STRU                            *pstHplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength        = 0;


    stManualSearchHplmnFlg.usManualSearchHplmnFlg = VOS_TRUE;

    pstHplmnCfgInfo = NAS_MML_GetHplmnCfg();

    NV_GetLength(en_NV_Item_SrchHplmnFlg_StartInManualMode, &ulLength);
    if (ulLength > sizeof(stManualSearchHplmnFlg))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_SrchHplmnFlg_StartInManualMode,
                         &(stManualSearchHplmnFlg), ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSearchHPlmnFlgManualModeNvim():en_NV_Item_SrchHplmnFlg_StartInManualMode Error");
        return;
    }

    if ((NAS_MMC_NV_ITEM_ACTIVE != stManualSearchHplmnFlg.usManualSearchHplmnFlg)
     && (NAS_MMC_NV_ITEM_DEACTIVE != stManualSearchHplmnFlg.usManualSearchHplmnFlg))
    {
        return;
    }

    pstHplmnCfgInfo->ucManualSearchHplmnFlg = (VOS_UINT8)stManualSearchHplmnFlg.usManualSearchHplmnFlg;

    return;
}


VOS_VOID NAS_MMC_ReadSearchHplmnFlgPowerOnNvim(VOS_VOID)
{
    NAS_MMC_NVIM_HPLMN_SEARCH_POWERON_STRU                  stHplmnSearchPowerOn;
    NAS_MML_HPLMN_CFG_INFO_STRU                            *pstHplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength        = 0;


    stHplmnSearchPowerOn.ucHplmnSearchPowerOn = VOS_FALSE;

    pstHplmnCfgInfo = NAS_MML_GetHplmnCfg();

    NV_GetLength(en_NV_Item_SearchHplmnAtPowerOn, &ulLength);
    if (ulLength > sizeof(stHplmnSearchPowerOn))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_SearchHplmnAtPowerOn, &stHplmnSearchPowerOn, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSearchHplmnFlgPowerOnNvim():en_NV_Item_SearchHplmnAtPowerOn Error");
        return;
    }

    if ((NAS_MMC_NV_ITEM_ACTIVE != stHplmnSearchPowerOn.ucHplmnSearchPowerOn)
     && (NAS_MMC_NV_ITEM_DEACTIVE != stHplmnSearchPowerOn.ucHplmnSearchPowerOn))
    {
        return;
    }

    pstHplmnCfgInfo->ucHplmnSearchPowerOn = stHplmnSearchPowerOn.ucHplmnSearchPowerOn;

    return;
}



VOS_VOID NAS_MMC_ReadEnhancedHPlmnSearchFlagNvim(VOS_VOID)
{
    NAS_MML_BG_SEARCH_CFG_INFO_STRU                        *pstBgSearchCfg = VOS_NULL_PTR;
    NAS_MMC_NVIM_ENHANCED_HPLMN_SRCH_FLG_STRU               stEnhancedHplmnSrchFlg;
    VOS_UINT32                                              ulLength;

    ulLength        = 0;


    stEnhancedHplmnSrchFlg.usEnhancedHplmnSrchFlg = VOS_FALSE;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();
    NV_GetLength(en_NV_Item_Enhanced_Hplmn_Srch_Flg, &ulLength);
    if (ulLength > sizeof(stEnhancedHplmnSrchFlg))
    {
        return;
    }
    if (NV_OK != NV_Read(en_NV_Item_Enhanced_Hplmn_Srch_Flg, &stEnhancedHplmnSrchFlg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,"NAS_MMC_ReadEnhancedHPlmnSearchFlagNvim(): en_NV_Item_Enhanced_Hplmn_Srch_Flg Error");
        return;
    }

    if ((NAS_MMC_NV_ITEM_ACTIVE != stEnhancedHplmnSrchFlg.usEnhancedHplmnSrchFlg)
     && (NAS_MMC_NV_ITEM_DEACTIVE != stEnhancedHplmnSrchFlg.usEnhancedHplmnSrchFlg))
    {
        return;
    }

    pstBgSearchCfg->ucEnhancedHplmnSrchFlg = (VOS_UINT8)stEnhancedHplmnSrchFlg.usEnhancedHplmnSrchFlg;

    return;
}


VOS_VOID NAS_MMC_ReadSpecialRoamFlgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_SPECIAL_ROAM_STRU      stSpecialRoam;
    NAS_MML_BG_SEARCH_CFG_INFO_STRU    *pstBgSearchCfg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength        = 0;


    stSpecialRoam.ucStatus         = NAS_MMC_NV_ITEM_DEACTIVE;
    stSpecialRoam.ucSpecialRoamFlg = VOS_FALSE;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();
    NV_GetLength(en_NV_Item_Special_Roam_Flag, &ulLength);
    if (ulLength > sizeof(stSpecialRoam))
    {
        return;
    }
    if (NV_OK != NV_Read (en_NV_Item_Special_Roam_Flag, &stSpecialRoam, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSpecialRoamFlgNvim():en_NV_Item_Special_Roam_Flag Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stSpecialRoam.ucStatus)
    {
        pstBgSearchCfg->ucSpecialRoamFlg = stSpecialRoam.ucSpecialRoamFlg;
    }

    return;
}
VOS_VOID NAS_MMC_ReadNetworkNameNvim( VOS_VOID )
{
    NAS_MMC_NVIM_OPERATOR_NAME_INFO_STRU                    stNvimOperatorName;
    NAS_MML_OPERATOR_NAME_INFO_STRU                        *pstOperatorName;

    pstOperatorName = NAS_MML_GetOperatorNameInfo();

    if(NV_OK != NV_Read (en_NV_Item_Network_Name_From_MM_Info, &stNvimOperatorName, sizeof(stNvimOperatorName)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadNetworkNameNvim:WARNING: Read NV Item Fail.");
        return;
    }

    pstOperatorName->stOperatorPlmnId.ulMcc = stNvimOperatorName.stOperatorPlmnId.ulMcc;
    pstOperatorName->stOperatorPlmnId.ulMnc = stNvimOperatorName.stOperatorPlmnId.ulMnc;
    PS_MEM_CPY(pstOperatorName->aucOperatorNameLong, stNvimOperatorName.aucOperatorNameLong, NAS_NVIM_MAX_OPER_LONG_NAME_LEN);
    PS_MEM_CPY(pstOperatorName->aucOperatorNameShort, stNvimOperatorName.aucOperatorNameShort, NAS_NVIM_MAX_OPER_SHORT_NAME_LEN);

}


VOS_UINT32 NAS_MMC_IsInNvEplmnList(
    NVIM_PLMN_VALUE_STRU               *pstPlmnId,
    VOS_UINT8                           ucPlmnNum,
    NVIM_PLMN_VALUE_STRU               *pstPlmnIdList
)
{
    VOS_UINT32                          i;

    for ( i = 0 ; i < ucPlmnNum ; i++ )
    {
        if ( (pstPlmnId->ucMcc[0] == pstPlmnIdList[i].ucMcc[0])
          && (pstPlmnId->ucMcc[1] == pstPlmnIdList[i].ucMcc[1])
          && (pstPlmnId->ucMcc[2] == pstPlmnIdList[i].ucMcc[2])
          && (pstPlmnId->ucMnc[0] == pstPlmnIdList[i].ucMnc[0])
          && (pstPlmnId->ucMnc[1] == pstPlmnIdList[i].ucMnc[1])
          && (pstPlmnId->ucMnc[2] == pstPlmnIdList[i].ucMnc[2]) )
        {
            return VOS_TRUE;
        }
    }
    return VOS_FALSE;
}




VOS_VOID NAS_MMC_ConvertMmlPlmnIdToNvimEquPlmn(
    NAS_MML_PLMN_ID_STRU               *pstMmlPlmnId,
    NVIM_PLMN_VALUE_STRU               *pstNvimPlmnId
)
{
    /* 转化MCC */
    pstNvimPlmnId->ucMcc[0] = (VOS_UINT8)(pstMmlPlmnId->ulMcc & NAS_MML_OCTET_LOW_FOUR_BITS);

    pstNvimPlmnId->ucMcc[1] = (VOS_UINT8)((pstMmlPlmnId->ulMcc >> 8) & NAS_MML_OCTET_LOW_FOUR_BITS);

    pstNvimPlmnId->ucMcc[2] = (VOS_UINT8)((pstMmlPlmnId->ulMcc >> 16) & NAS_MML_OCTET_LOW_FOUR_BITS);

    /* 转化MNC */
    pstNvimPlmnId->ucMnc[0] = (VOS_UINT8)(pstMmlPlmnId->ulMnc & NAS_MML_OCTET_LOW_FOUR_BITS);

    pstNvimPlmnId->ucMnc[1] = (VOS_UINT8)((pstMmlPlmnId->ulMnc >> 8) & NAS_MML_OCTET_LOW_FOUR_BITS);

    pstNvimPlmnId->ucMnc[2] = (VOS_UINT8)((pstMmlPlmnId->ulMnc >> 16) & NAS_MML_OCTET_LOW_FOUR_BITS);

}


VOS_VOID NAS_MMC_ConvertMmlEquListToNvimEquPlmnList(
    NAS_MML_EQUPLMN_INFO_STRU          *pstMmlEPlmnList,
    NVIM_EQUIVALENT_PLMN_LIST_STRU     *pstNvimEPlmnList
)
{
    VOS_UINT8                           i;

    pstNvimEPlmnList->ucCount = pstMmlEPlmnList->ucEquPlmnNum;

    if (pstNvimEPlmnList->ucCount > NVIM_MAX_EPLMN_NUM)
    {
        pstNvimEPlmnList->ucCount = NVIM_MAX_EPLMN_NUM;
    }

    for (i = 0; i < pstNvimEPlmnList->ucCount; i++)
    {
        NAS_MMC_ConvertMmlPlmnIdToNvimEquPlmn(&(pstMmlEPlmnList->astEquPlmnAddr[i]),
                                              &(pstNvimEPlmnList->struPlmnList[i]));

    }

    return;
}


VOS_UINT32 NAS_MMC_IsNvimEPlmnListEqual(
    NVIM_EQUIVALENT_PLMN_LIST_STRU     *pstNvimEPlmnList1,
    NVIM_EQUIVALENT_PLMN_LIST_STRU     *pstNvimEPlmnList2
)
{
    VOS_UINT32                          i;

    if (pstNvimEPlmnList1->ucCount != pstNvimEPlmnList2->ucCount)
    {
        return VOS_FALSE;
    }

    /* Eplmn个数大于0,但Rplmn不同时，EplmnList不相同 */
    if (pstNvimEPlmnList1->ucCount > 0)
    {
        if (VOS_FALSE == NAS_MMC_IsInNvEplmnList(&(pstNvimEPlmnList1->struPlmnList[0]),
											     1,
                                                 pstNvimEPlmnList2->struPlmnList))
        {
            return VOS_FALSE;
        }
    }
    else
    {
        return VOS_TRUE;
    }

    /* 其它Eplmn没有进行排序且有重复数据，需要对比2个列表才能确定相同 */
    for ( i = 1 ; i < pstNvimEPlmnList1->ucCount ; i++ )
    {
        if (VOS_FALSE == NAS_MMC_IsInNvEplmnList(&(pstNvimEPlmnList1->struPlmnList[i]),
											     pstNvimEPlmnList2->ucCount,
                                                 pstNvimEPlmnList2->struPlmnList))
        {
            return VOS_FALSE;
        }
    }

    for ( i = 1 ; i < pstNvimEPlmnList2->ucCount ; i++ )
    {
        if (VOS_FALSE == NAS_MMC_IsInNvEplmnList(&(pstNvimEPlmnList2->struPlmnList[i]),
                                                 pstNvimEPlmnList1->ucCount,
                                                 pstNvimEPlmnList1->struPlmnList))
        {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}


VOS_VOID NAS_MMC_DeleteEPlmnList( VOS_VOID )
{
    NAS_MML_EQUPLMN_INFO_STRU          *pstEPlmnList    = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU               *pstCurrPlmnId = VOS_NULL_PTR;

    pstEPlmnList   = NAS_MML_GetEquPlmnList();

    /* 删除Eplmn */
    NAS_MML_InitEquPlmnInfo(pstEPlmnList);

    /* 更新NV的Eplmn */
    NAS_MMC_WriteEplmnNvim();

    /*将当前网络添加Eplmn列表*/
    pstCurrPlmnId = NAS_MML_GetCurrCampPlmnId();

    pstEPlmnList->astEquPlmnAddr[0].ulMcc = pstCurrPlmnId->ulMcc;
    pstEPlmnList->astEquPlmnAddr[0].ulMnc = pstCurrPlmnId->ulMnc;

    pstEPlmnList->ucEquPlmnNum = 1;

    return;
}
VOS_VOID NAS_MMC_WriteEplmnNvim(VOS_VOID)
{
    VOS_UINT32                          ulUpdateNvFlag;
    NVIM_EQUIVALENT_PLMN_LIST_STRU     *pstNewNvEquPlmnList = VOS_NULL_PTR;
    NVIM_EQUIVALENT_PLMN_LIST_STRU     *pstOldNvEquPlmnList = VOS_NULL_PTR;


    ulUpdateNvFlag  = VOS_FALSE;

    pstNewNvEquPlmnList = (NVIM_EQUIVALENT_PLMN_LIST_STRU*)PS_MEM_ALLOC(
                                                      WUEPS_PID_MMC,
                                                      sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU));

    if (VOS_NULL_PTR == pstNewNvEquPlmnList)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_WriteEplmnNvim:ERROR: Memory Alloc Error");
        return;
    }

    pstOldNvEquPlmnList = (NVIM_EQUIVALENT_PLMN_LIST_STRU*)PS_MEM_ALLOC(
                                                        WUEPS_PID_MMC,
                                                        sizeof (NVIM_EQUIVALENT_PLMN_LIST_STRU));

    if (VOS_NULL_PTR == pstOldNvEquPlmnList)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_WriteEplmnNvim:ERROR: Memory Alloc Error");
        PS_MEM_FREE(WUEPS_PID_MMC, pstNewNvEquPlmnList);
        return;
    }

    PS_MEM_SET(pstNewNvEquPlmnList, 0, sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU));
    PS_MEM_SET(pstOldNvEquPlmnList, 0, sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU));

    NAS_MMC_ConvertMmlEquListToNvimEquPlmnList(NAS_MML_GetEquPlmnList(),
                                               pstNewNvEquPlmnList);

    /* 读取NV中EPLMN信息 */
    if ( NV_OK == NV_Read(en_NV_Item_EquivalentPlmn,
                         pstOldNvEquPlmnList, sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU)) )
    {
        if (VOS_FALSE == NAS_MMC_IsNvimEPlmnListEqual(pstNewNvEquPlmnList,
                                                      pstOldNvEquPlmnList))
        {
            ulUpdateNvFlag = VOS_TRUE;
        }
    }
    else
    {
        /* 读取 NV 失败，需要更新 EPLMN */
        ulUpdateNvFlag = VOS_TRUE;
    }

    if ( VOS_TRUE == ulUpdateNvFlag )
    {
        if (NV_OK != NV_Write(en_NV_Item_EquivalentPlmn,
                              pstNewNvEquPlmnList,
                              sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU)))
        {
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_WriteEplmnNvim:WARNING: en_NV_Item_EquivalentPlmn Error");
        }
    }

    PS_MEM_FREE(WUEPS_PID_MMC, pstNewNvEquPlmnList);
    PS_MEM_FREE(WUEPS_PID_MMC, pstOldNvEquPlmnList);


}
VOS_VOID NAS_MMC_ReadHplmnFirstSearchPeriodNvim(VOS_VOID)
{
    NAS_NVIM_HPLMN_FIRST_TIMER_STRU stFirstHplmnTime;
    NAS_MML_BG_SEARCH_CFG_INFO_STRU    *pstBgSearchCfg = VOS_NULL_PTR;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();

    /* 初始默认为2分钟 */
    stFirstHplmnTime.ucHplmnTimerLen = 2;
    if (NV_OK != NV_Read (en_NV_Item_HPlmnFirstTimer, &(stFirstHplmnTime.ucHplmnTimerLen), sizeof(VOS_UINT8)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadHplmnFirstSearchPeriodNvim():WARNING: en_NV_Item_HPlmnFirstTimer Error");
        return;
    }

    /* NV项中是以分钟来保存的,实际上启动定时器时需要以ms来启动 */
    pstBgSearchCfg->ulFirstStartHplmnTimerLen = (stFirstHplmnTime.ucHplmnTimerLen) * 60 *1000;

    return;
}


VOS_VOID NAS_MMC_ReadDefaultHplmnSrchPeriodNvim(VOS_VOID)
{
    NAS_MMC_NVIM_DEFAULT_MAX_HPLMN_PERIOD_STRU   stDefaultHplmnSrchPeri;
    NAS_MML_BG_SEARCH_CFG_INFO_STRU             *pstBgSearchCfg = VOS_NULL_PTR;


    stDefaultHplmnSrchPeri.ucDefaultMaxHplmnPeriFlg = NAS_MMC_NV_ITEM_DEACTIVE;
    stDefaultHplmnSrchPeri.ucDefaultMaxHplmnTim     = 0;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();

    if (NV_OK != NV_Read(en_NV_Item_Default_Max_Hplmn_Srch_Peri,
                         &stDefaultHplmnSrchPeri,
                         sizeof(NAS_MMC_NVIM_DEFAULT_MAX_HPLMN_PERIOD_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadDefaultHplmnSrchPeriodNvim(): en_NV_Item_Default_Hplmn_Srch_Peri Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stDefaultHplmnSrchPeri.ucDefaultMaxHplmnPeriFlg)
    {
        pstBgSearchCfg->stDefaultMaxHplmnTimerPeriod.ucNvimActiveFlg      = stDefaultHplmnSrchPeri.ucDefaultMaxHplmnPeriFlg;

        /* NVIM中是以分钟来保存的,启动定时器需要以ms来启动 */
        pstBgSearchCfg->stDefaultMaxHplmnTimerPeriod.ulDefaultMaxHplmnLen = stDefaultHplmnSrchPeri.ucDefaultMaxHplmnTim * 60 * 1000;
    }

    return;
}

/* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
/*****************************************************************************
 函 数 名  : NAS_MMC_ReadHighPrioPlmnRefreshTriggerBGSearchFlagNvim
 功能描述  : 读取en_NV_Item_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_CFG nv项内容
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年04月28日
    作    者   : c00318887
    修改内容   : for file refresh 新增
*****************************************************************************/
VOS_VOID NAS_MMC_ReadHighPrioPlmnRefreshTriggerBGSearchFlagNvim(VOS_VOID)
{
    NAS_MMC_NVIM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_STRU  stHighPrioPlmnRefreshTriggerBGSearchCfg;
    VOS_UINT32                                                  ulLength;

    ulLength = 0;
    PS_MEM_SET(&stHighPrioPlmnRefreshTriggerBGSearchCfg, 0x0, sizeof(NAS_MMC_NVIM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_STRU));

    /* 先获取NV的长度 */
    (VOS_VOID)NV_GetLength(en_NV_Item_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_CFG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadHighPrioPlmnRefreshTriggerBGSearchFlagNvim(): en_NV_Item_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_CFG length Error");
        return;
    }


    if (NV_OK != NV_Read (en_NV_Item_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_CFG, &stHighPrioPlmnRefreshTriggerBGSearchCfg, sizeof(NAS_MMC_NVIM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadHighPrioPlmnRefreshTriggerBGSearchFlagNvim():WARNING: en_NV_Item_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_CFG Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stHighPrioPlmnRefreshTriggerBGSearchCfg.ucTriggerBGSearchFlag)
    {
        NAS_MML_SetHighPrioPlmnRefreshTriggerBGSearchFlag(stHighPrioPlmnRefreshTriggerBGSearchCfg.ucTriggerBGSearchFlag);
        
        /* NVIM中是以秒来保存的,启动定时器需要以ms来启动 */
        if (0 == stHighPrioPlmnRefreshTriggerBGSearchCfg.usSearchDelayLen)
        {
            stHighPrioPlmnRefreshTriggerBGSearchCfg.usSearchDelayLen = NV_ITEM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_DEFAULT_DELAY_LEN;
        }
        
        NAS_MML_SetHighPrioPlmnRefreshTriggerBGSearchDelayLen(stHighPrioPlmnRefreshTriggerBGSearchCfg.usSearchDelayLen * 1000);
    }

    return;
}

/* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */

VOS_VOID NAS_MMC_ReadSrchHplmnTtimerValueNvim(VOS_VOID)
{
    NAS_MMC_NVIM_SEARCH_HPLMN_TIMER_STRU stSrchHplmnTimer;
    NAS_MML_BG_SEARCH_CFG_INFO_STRU     *pstBgSearchCfg = VOS_NULL_PTR;


    stSrchHplmnTimer.ucStatus              = NAS_MMC_NV_ITEM_DEACTIVE;
    stSrchHplmnTimer.usSrchHplmnTimerValue = 0;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();

    if (NV_OK != NV_Read (en_NV_Item_SearchHplmnTtimerValue, &stSrchHplmnTimer, sizeof(NAS_MMC_NVIM_SEARCH_HPLMN_TIMER_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadSrchHplmnTtimerValueNvim():WARNING: en_NV_Item_SearchHplmnTtimerValue Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stSrchHplmnTimer.ucStatus)
    {
        pstBgSearchCfg->stNonFirstHplmnTimerPeriod.ucNvimActiveFlg         = stSrchHplmnTimer.ucStatus;
        pstBgSearchCfg->stNonFirstHplmnTimerPeriod.ulNonFirstHplmnTimerLen = stSrchHplmnTimer.usSrchHplmnTimerValue * 60 * 1000;
    }

    return;
}




VOS_VOID NAS_MMC_ReadWhiteOpeLockPlmnInfoNvim( VOS_VOID )
{
    NAS_MMC_NVIM_OPER_LOCK_WHITEPLMN_STRU stOperLockPlmnList;
    NAS_MML_PLMN_LOCK_CFG_INFO_STRU      *pstPlmnLockCfgInfo = VOS_NULL_PTR;


    stOperLockPlmnList.ulWhitePlmnLockNum = 0;

    pstPlmnLockCfgInfo = NAS_MML_GetPlmnLockCfg();

    if (NV_OK != NV_Read (en_NV_Item_OPERLOCK_PLMN_INFO_WHITE, &stOperLockPlmnList, sizeof(NAS_MMC_NVIM_OPER_LOCK_WHITEPLMN_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadWhiteOpeLockPlmnInfoNvim:Read NV Failed");
        return;
    }

    if (0 != stOperLockPlmnList.ulWhitePlmnLockNum)
    {
        /* 白名单生效 */
        pstPlmnLockCfgInfo->ucWhitePlmnLockNum = (VOS_UINT8)stOperLockPlmnList.ulWhitePlmnLockNum;

        if (stOperLockPlmnList.ulWhitePlmnLockNum > NAS_MML_MAX_WHITE_LOCK_PLMN_NUM)
        {
            pstPlmnLockCfgInfo->ucWhitePlmnLockNum = NAS_MML_MAX_WHITE_LOCK_PLMN_NUM;
        }

        PS_MEM_CPY(pstPlmnLockCfgInfo->astWhitePlmnId,
                   stOperLockPlmnList.astWhitePlmnId,
                   (sizeof(NAS_MML_PLMN_ID_STRU) * NAS_MML_MAX_WHITE_LOCK_PLMN_NUM));
    }

    return;
}
VOS_VOID NAS_MMC_ReadBlackOperLockPlmnInfoNvim( VOS_VOID )
{
    NAS_MMC_NVIM_OPER_LOCK_BLACKPLMN_STRU stOperLockPlmnList;
    NAS_MML_PLMN_LOCK_CFG_INFO_STRU      *pstPlmnLockCfgInfo = VOS_NULL_PTR;


    stOperLockPlmnList.ulBlackPlmnLockNum = 0;


    PS_MEM_SET(&stOperLockPlmnList, 0x00, sizeof(stOperLockPlmnList));

    pstPlmnLockCfgInfo = NAS_MML_GetPlmnLockCfg();

    if (NV_OK != NV_Read (en_NV_Item_OperLock_Plmn_Info, &stOperLockPlmnList, sizeof(NAS_MMC_NVIM_OPER_LOCK_BLACKPLMN_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadBlackOperLockPlmnInfoNvim:Read NV Failed");
        return;
    }

    pstPlmnLockCfgInfo->ucBlackPlmnLockNum = (VOS_UINT8)stOperLockPlmnList.ulBlackPlmnLockNum;

    if ( stOperLockPlmnList.ulBlackPlmnLockNum > NAS_MML_MAX_BLACK_LOCK_PLMN_NUM )
    {
        pstPlmnLockCfgInfo->ucBlackPlmnLockNum = NAS_MML_MAX_BLACK_LOCK_PLMN_NUM;
    }

    PS_MEM_CPY(pstPlmnLockCfgInfo->astBlackPlmnId,
               stOperLockPlmnList.astBlackPlmnId,
               (sizeof(NAS_MML_PLMN_ID_STRU) * NAS_MML_MAX_BLACK_LOCK_PLMN_NUM));

    return;
}


VOS_UINT32 NAS_MMC_IsRatForbiddenListNvimParaRangeValid(  
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_STRU *pstParaValue
)
{
    if (NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_BUTT <= pstParaValue->enSwitchFlag)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_IsRatForbiddenListNvimValid:The para enSwitchFlag is invalid");
        return VOS_FALSE;
    }

    /* 参数检查，如果NV配置的SIM卡数目大于NVIM_MAX_IMSI_FORBIDDEN_LIST_NUM，则认为配置的NV参数非法，直接返回；*/
    if ( NAS_NVIM_MAX_IMSI_FORBIDDEN_LIST_NUM < pstParaValue->ucImsiListNum )
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_IsRatForbiddenListNvimValid:The para ucImsiListNum is invalid");
        return VOS_FALSE;
    }
    
    /* 参数检查，如果NV配置的禁止RAT数目大于2，则认为配置的NV参数非法，直接返回 */
    if ( NAS_NVIM_MAX_SUPPORTED_FORBIDDEN_RAT_NUM < pstParaValue->ucForbidRatNum )
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_IsRatForbiddenListNvimValid:The para ucForbidRatNum is invalid");
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT8 NAS_MMC_IsRatForbiddenListNvimValid( 
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_STRU *pstParaValue
)
{
    VOS_UINT32                          i;

    i = 0;

    if ( VOS_FALSE == NAS_MMC_IsRatForbiddenListNvimParaRangeValid(pstParaValue) )
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_IsRatForbiddenListNvimValid:The para is invalid");
        return VOS_FALSE;
    }

    /* 如果禁止网络制式列表中包含GSM，则返回VOS_FALSE */
    for (i = 0; i < pstParaValue->ucForbidRatNum; i++)
    {
        if (NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_GERAN == pstParaValue->aenForbidRatList[i])
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_IsRatForbiddenListNvimValid:The para aenForbidRatList is invalid");
            return VOS_FALSE;
        }
    }
    
    /* 如果禁止接入技术的数目为0，也认为参数是正确的 */
    if (NAS_NVIM_FORBIDDEN_RAT_NUM_0 == pstParaValue->ucForbidRatNum)
    {
        return VOS_TRUE;
    }

    /* 只允许两种组合，仅LTE或LTE+UTRAN */
    if ((NAS_NVIM_FORBIDDEN_RAT_NUM_1             == pstParaValue->ucForbidRatNum) 
     && (NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN == pstParaValue->aenForbidRatList[0]))
    {
        return VOS_TRUE;
    }

    if ((NAS_NVIM_FORBIDDEN_RAT_NUM_2 == pstParaValue->ucForbidRatNum))
    {
        if ((NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_UTRAN  == pstParaValue->aenForbidRatList[0])
         && (NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN == pstParaValue->aenForbidRatList[1]))
        {
            return VOS_TRUE;
        }

        if ((NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN == pstParaValue->aenForbidRatList[0])
         && (NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_UTRAN  == pstParaValue->aenForbidRatList[1]))
        {
            return VOS_TRUE;
        }

        if ((NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN == pstParaValue->aenForbidRatList[0])
         && (NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN == pstParaValue->aenForbidRatList[1]))
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
    
}
VOS_UINT32 NAS_MMC_ConvertNvimRatToMmlType( 
    NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_ENUM_UINT8            enNvRatType,
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8                        *penMmlRatType
)
{
    switch(enNvRatType)
    {
        case NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_GERAN:
            *penMmlRatType = NAS_MML_NET_RAT_TYPE_GSM;
            break;

        case NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_UTRAN:
            *penMmlRatType = NAS_MML_NET_RAT_TYPE_WCDMA;
            break;

        case NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN:
            *penMmlRatType = NAS_MML_NET_RAT_TYPE_LTE;
            break;

        default:
            *penMmlRatType = NAS_MML_NET_RAT_TYPE_BUTT;
            return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_VOID NAS_MMC_ReadRatForbiddenListConfigNvim( VOS_VOID )
{
    VOS_UINT32                                              i;
    NAS_MML_SIM_FORMAT_PLMN_ID                              stSimPlmn;
    NAS_MML_PLMN_ID_STRU                                    stPlmn;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8                         enMmlRatType;

    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_STRU                    stRatForbiddenListCfgNvim;
    NAS_MML_RAT_FORBIDDEN_LIST_STRU                        *pstRatForbiddenList = VOS_NULL_PTR;

    PS_MEM_SET(&stRatForbiddenListCfgNvim, 0x00, sizeof(stRatForbiddenListCfgNvim));
    PS_MEM_SET(&stSimPlmn, 0x00, sizeof(stSimPlmn));
    PS_MEM_SET(&stPlmn, 0x00, sizeof(stPlmn));

    enMmlRatType = NAS_MML_NET_RAT_TYPE_BUTT;

    pstRatForbiddenList = NAS_MML_GetRatForbiddenListCfg();

    if (NV_OK != NV_Read(en_NV_Item_Rat_Forbidden_List_Config, &stRatForbiddenListCfgNvim, sizeof(NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRatForbiddenListAccordImsiConfigNvim:Read NV Failed");
        return;
    }

    /* 统一做参数检查 */
    if (VOS_FALSE == NAS_MMC_IsRatForbiddenListNvimValid(&stRatForbiddenListCfgNvim))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRatForbiddenListAccordImsiConfigNvim:The paras of nvim is invalid");
        return;

    }
    /* 将NV中的参数保存到MML */
    pstRatForbiddenList->enSwitchFlag   = stRatForbiddenListCfgNvim.enSwitchFlag;

    /* 保存禁止RAT信息 */
    pstRatForbiddenList->ucForbidRatNum = 0;
    for (i = 0; i < stRatForbiddenListCfgNvim.ucForbidRatNum; i++)
    {
        if (VOS_TRUE == NAS_MMC_ConvertNvimRatToMmlType(stRatForbiddenListCfgNvim.aenForbidRatList[i], &enMmlRatType))
        {
            pstRatForbiddenList->aenForbidRatList[pstRatForbiddenList->ucForbidRatNum] = enMmlRatType;
            pstRatForbiddenList->ucForbidRatNum++;
        }
    }

    pstRatForbiddenList->ucImsiListNum  = 0;

    /* 将USIM中保存的PLMN信息转换为内存中的信息 */
    for ( i = 0; i < stRatForbiddenListCfgNvim.ucImsiListNum; i++ )
    {
        PS_MEM_CPY(stSimPlmn.aucSimPlmn, stRatForbiddenListCfgNvim.astImsiList[i].aucSimPlmn, NAS_MML_SIM_PLMN_ID_LEN);
        NAS_MMC_ConvertSimPlmnToNasPLMN(&stSimPlmn, &stPlmn);

        if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stPlmn))
        {
            pstRatForbiddenList->astImsiList[pstRatForbiddenList->ucImsiListNum].ulMcc = stPlmn.ulMcc;
            pstRatForbiddenList->astImsiList[pstRatForbiddenList->ucImsiListNum].ulMnc = stPlmn.ulMnc;
            pstRatForbiddenList->ucImsiListNum++;
        }
    }
     
    return;
}



VOS_VOID NAS_MMC_ReadDisabledRatPlmnInfoNvim( VOS_VOID )
{
    NAS_MMC_NVIM_DISABLED_RAT_PLMN_INFO_STRU                stDisabledRatPlmnList;

    NAS_MML_DISABLED_RAT_PLMN_CFG_INFO_STRU                *pstDisabledRatPlmnCfgInfo;
    VOS_UINT32                                              i;


    PS_MEM_SET(&stDisabledRatPlmnList, 0x00, sizeof(stDisabledRatPlmnList));

    pstDisabledRatPlmnCfgInfo = NAS_MML_GetDisabledRatPlmnCfg();

    if (NV_OK != NV_Read (en_NV_Item_Disabled_Rat_Plmn_Info, &stDisabledRatPlmnList, sizeof(NAS_MMC_NVIM_DISABLED_RAT_PLMN_INFO_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDisabledRatPlmnInfoNvim:Read NV Failed");
        return;
    }

    pstDisabledRatPlmnCfgInfo->ulDisabledRatPlmnNum = (VOS_UINT8)stDisabledRatPlmnList.ulDisabledRatPlmnNum;

    if (stDisabledRatPlmnList.ulDisabledRatPlmnNum > NAS_MML_NVIM_MAX_DISABLED_RAT_PLMN_NUM)
    {
        pstDisabledRatPlmnCfgInfo->ulDisabledRatPlmnNum = NAS_MML_NVIM_MAX_DISABLED_RAT_PLMN_NUM;
    }

    for (i = 0; i < pstDisabledRatPlmnCfgInfo->ulDisabledRatPlmnNum; i++)
    {
        pstDisabledRatPlmnCfgInfo->astDisabledRatPlmnId[i].stPlmnWithRat.stPlmnId.ulMcc = stDisabledRatPlmnList.astDisabledRatPlmnId[i].ulMcc;
        pstDisabledRatPlmnCfgInfo->astDisabledRatPlmnId[i].stPlmnWithRat.stPlmnId.ulMnc = stDisabledRatPlmnList.astDisabledRatPlmnId[i].ulMnc;
        pstDisabledRatPlmnCfgInfo->astDisabledRatPlmnId[i].stPlmnWithRat.enRat          = stDisabledRatPlmnList.astDisabledRatPlmnId[i].enRat;
        pstDisabledRatPlmnCfgInfo->astDisabledRatPlmnId[i].ulPlmnDisabledExpiredSlice   = NAS_MML_MAX_TIME_SLICE;
        pstDisabledRatPlmnCfgInfo->astDisabledRatPlmnId[i].ulPlmnDisabledBeginSlice     = 0;
    }

    return;
}


VOS_VOID NAS_MMC_ReadUserCfgExtEhplmnInfoNvim( VOS_VOID )
{
    VOS_UINT32                                              i;
    VOS_UINT32                                              j;
    VOS_UINT32                                              ulNvimEhplmnNum;
    NAS_MML_SIM_FORMAT_PLMN_ID                              stSimPlmn;
    NAS_MML_PLMN_ID_STRU                                    stPlmn;
    NAS_MMC_NVIM_USER_CFG_EXT_EHPLMN_INFO_STRU              stNvimCfgExtEhplmnInfo;
    NAS_MML_USER_CFG_EXT_EHPLMN_INFO_STRU                  *pstUserCfgExtEhplmnInfo;

    PS_MEM_SET(&stNvimCfgExtEhplmnInfo, 0x00, sizeof(stNvimCfgExtEhplmnInfo));

    pstUserCfgExtEhplmnInfo = NAS_MML_GetUserCfgExtEhplmnInfo();
    /*循环变量初始化*/
    ulNvimEhplmnNum = 0;
    
    if ( NV_OK != NV_Read( en_NV_Item_User_Cfg_Ext_Ehplmn_Info,
                            &stNvimCfgExtEhplmnInfo, sizeof(NAS_MMC_NVIM_USER_CFG_EXT_EHPLMN_INFO_STRU)) )
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadUserCfgEhplmnInfoNvim:Read NV Failed");
        return;
    }
    
    ulNvimEhplmnNum = stNvimCfgExtEhplmnInfo.ulNvimEhplmnNum;
    pstUserCfgExtEhplmnInfo->ulUserCfgEhplmnNum = ulNvimEhplmnNum;
    if ( ulNvimEhplmnNum > NAS_MML_MAX_USER_CFG_EXT_EHPLMN_NUM )
    {
        ulNvimEhplmnNum = NAS_MML_MAX_USER_CFG_EXT_EHPLMN_NUM;
    }
    /* 将NV中的PLMN LIST信息保存到MML的全局变量中去 */
    for (j = 0; j < ulNvimEhplmnNum; j++)
    {
        /* 初始化内存中存放PLMN网络列表个数 */
        pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].ucImsiPlmnListNum = 0;        

        /* 防止数组越界 */
        if ( stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].ucImsiPlmnListNum > NAS_MMC_MAX_USER_CFG_IMSI_PLMN_NUM )
        {
            stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].ucImsiPlmnListNum = NAS_MMC_MAX_USER_CFG_IMSI_PLMN_NUM;
        }

        for ( i = 0; i < stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].ucImsiPlmnListNum; i++ )
        {
            PS_MEM_CPY(stSimPlmn.aucSimPlmn, stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].astImsiPlmnList[i].aucSimPlmn, NAS_MML_SIM_PLMN_ID_LEN);
            NAS_MMC_ConvertSimPlmnToNasPLMN(&stSimPlmn, &stPlmn);

            if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stPlmn))
            {
                pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].astImsiPlmnList[pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].ucImsiPlmnListNum] = stPlmn;
                pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].ucImsiPlmnListNum++;
            }
        }

        /* 将NV中的EHplmn信息保存到MML的全局变量中去 */

        /* 初始化内存中存放PLMN网络列表个数 */
        pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].ucEhplmnListNum = 0;

        /* 防止数组越界 */
        if ( stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].ucEhplmnListNum > NAS_MMC_MAX_USER_CFG_EHPLMN_NUM )
        {
            stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].ucEhplmnListNum = NAS_MMC_MAX_USER_CFG_EHPLMN_NUM;
        }

        for ( i = 0; i < stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].ucEhplmnListNum; i++ )
        {
            PS_MEM_CPY(stSimPlmn.aucSimPlmn, stNvimCfgExtEhplmnInfo.astNvimEhplmnInfo[j].astEhPlmnList[i].aucSimPlmn, NAS_MML_SIM_PLMN_ID_LEN);
            NAS_MMC_ConvertSimPlmnToNasPLMN(&stSimPlmn, &stPlmn);

            if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stPlmn))
            {
                pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].astEhPlmnList[pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].ucEhplmnListNum] = stPlmn;
                pstUserCfgExtEhplmnInfo->astUserCfgEhplmnInfo[j].ucEhplmnListNum++;
            }
        }
    }

    return;
}


VOS_VOID NAS_MMC_ReadAvailableTimerInfoNvim( VOS_VOID  )
{
    NAS_MML_AVAIL_TIMER_CFG_STRU       *pstAvailTimerCfg = VOS_NULL_PTR;
    NAS_MMC_NVIM_AVAIL_TIMER_CFG_STRU   stNvimAvailTimerCfg;


    PS_MEM_SET(&stNvimAvailTimerCfg, 0x00, sizeof(stNvimAvailTimerCfg));

    pstAvailTimerCfg = NAS_MML_GetAvailTimerCfg();

    if (NV_OK != NV_Read (en_NV_Item_SEARCH_TIMER_INFO, &stNvimAvailTimerCfg, sizeof(stNvimAvailTimerCfg)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadAvailableTimerInfoNvim:Read NV Failed");
        return;
    }

    pstAvailTimerCfg->ulFirstSearchTimeCount = stNvimAvailTimerCfg.ulFirstSearchTimeCount;
    pstAvailTimerCfg->ulFirstSearchTimeLen   = stNvimAvailTimerCfg.ulFirstSearchTimeLen;
    pstAvailTimerCfg->ulDeepSearchTimeCount  = stNvimAvailTimerCfg.ulDeepSearchTimeCount;
    pstAvailTimerCfg->ulDeepSearchTimeLen    = stNvimAvailTimerCfg.ulDeepSearchTimeLen;
    return;
}


VOS_VOID NAS_MMC_ReadStandardSTKSteeringOfRoamingFlgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_STK_STEERING_OF_ROAMING_SUPPORT_FLG_STRU   stStkSteeringOfRoamingSupportFlg;
    VOS_UINT32                                              ulLength;

    NV_GetLength(en_NV_Item_Standard_STK_SteeringOfRoaming_Support_Flg, &ulLength);
    stStkSteeringOfRoamingSupportFlg.usStkSteeringOfRoamingSupportFlg = VOS_TRUE;

    if (NV_OK != NV_Read(en_NV_Item_Standard_STK_SteeringOfRoaming_Support_Flg,
                         &stStkSteeringOfRoamingSupportFlg,
                         ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadStandardSTKSteeringOfRoamingFlgNvim():WARNING: en_NV_Item_Standard_STK_SteeringOfRoaming_Support_Flg Error");
        return;
    }

    if ((NAS_MMC_NV_ITEM_ACTIVE !=  stStkSteeringOfRoamingSupportFlg.usStkSteeringOfRoamingSupportFlg)
     && (NAS_MMC_NV_ITEM_DEACTIVE !=  stStkSteeringOfRoamingSupportFlg.usStkSteeringOfRoamingSupportFlg))
    {
        return;
    }

    NAS_MML_SetStkSteeringOfRoamingSupportFlg((VOS_UINT8) stStkSteeringOfRoamingSupportFlg.usStkSteeringOfRoamingSupportFlg);

    return;
}


VOS_VOID NAS_MMC_ReadCsRejSearchSupportFlagNvim(VOS_VOID)
{
    NAS_MMC_NVIM_CS_REJ_SEARCH_SUPPORT_STRU  stCsRejSearchSupportFlg;
    VOS_UINT32                               ulLength;

    ulLength        = 0;


    stCsRejSearchSupportFlg.ucStatus                = NAS_MMC_NV_ITEM_DEACTIVE;
    stCsRejSearchSupportFlg.ucCsRejSearchSupportFlg = VOS_FALSE;

    NV_GetLength(en_NV_Item_SteeringofRoaming_SUPPORT_CTRL, &ulLength);
    if (ulLength > sizeof(stCsRejSearchSupportFlg))
    {
        return;
    }

    if (NV_OK != NV_Read (en_NV_Item_SteeringofRoaming_SUPPORT_CTRL,
                          &stCsRejSearchSupportFlg, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCsRejSearchSupportFlagNvim():WARNING: NV_Read en_NV_Item_SteeringofRoaming_SUPPORT_CTRL Error");
        return;
    }

    if(NAS_MMC_NV_ITEM_ACTIVE == stCsRejSearchSupportFlg.ucStatus)
    {
        NAS_MML_SetCsRejSearchSupportFlg(stCsRejSearchSupportFlg.ucCsRejSearchSupportFlg);
    }

    return;
}


VOS_VOID NAS_MMC_ReadAutoAttachNvim(VOS_VOID)
{
    NAS_NVIM_AUTOATTACH_STRU            stAutoattachFlag;
    VOS_UINT32                          ulLength;

    ulLength        = 0;


    stAutoattachFlag.usAutoattachFlag= VOS_TRUE;

    NV_GetLength(en_NV_Item_Autoattach, &ulLength);
    if (ulLength > sizeof(stAutoattachFlag))
    {
        return;
    }

    if(NV_OK != NV_Read (en_NV_Item_Autoattach, &stAutoattachFlag, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadAutoAttachNvim():WARNING: en_NV_Item_Autoattach Error");
        return;
    }

    NAS_MML_SetPsAutoAttachFlg((VOS_UINT8)stAutoattachFlag.usAutoattachFlag);
    return;
}


VOS_VOID NAS_MMC_ReadPlmnSelectionModeNvim(VOS_VOID)
{
    NAS_NVIM_SELPLMN_MODE_STRU                          stPlmnSelMode;


    stPlmnSelMode.usSelPlmnMode = NAS_MMC_PLMN_SELECTION_MODE_BUTT;

    if (NV_OK != NV_Read (en_NV_Item_SelPlmn_Mode, &stPlmnSelMode.usSelPlmnMode, sizeof(stPlmnSelMode.usSelPlmnMode)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlmnSelectionModeNvim():WARNING: en_NV_Item_SelPlmn_Mode Error");
        return;
    }

    if ((stPlmnSelMode.usSelPlmnMode != NAS_MMC_PLMN_SELECTION_MODE_AUTO)
     && (stPlmnSelMode.usSelPlmnMode != NAS_MMC_PLMN_SELECTION_MODE_MANUAL))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlmnSelectionModeNvim():NV parameter invalid");
        return;
    }

    NAS_MMC_SetPlmnSelectionMode((NAS_MMC_PLMN_SELECTION_MODE_ENUM_UINT8)(stPlmnSelMode.usSelPlmnMode));

    return;
}
VOS_VOID NAS_MMC_WritePlmnSelectionModeNvim(VOS_VOID)
{
    NAS_NVIM_SELPLMN_MODE_STRU          stPlmnSelMode;
    VOS_UINT32                          ulNvLength;

    ulNvLength = 0;
    stPlmnSelMode.usSelPlmnMode = NAS_MMC_GetPlmnSelectionMode();

    NV_GetLength(en_NV_Item_SelPlmn_Mode, &ulNvLength);
    if (NV_OK != NV_Write(en_NV_Item_SelPlmn_Mode,
                      &stPlmnSelMode,
                      ulNvLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_WritePlmnSelectionModeNvim():en_NV_Item_SelPlmn_Mode Error");
    }

    return;
}


VOS_VOID NAS_MMC_ReadEquivalentPlmnNvim(VOS_VOID)
{

    NVIM_EQUIVALENT_PLMN_LIST_STRU      stEquivPlmn;
    VOS_UINT32                          i;
    NAS_MML_NETWORK_INFO_STRU          *pstNetWorkInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulEquPlmnNum;


    PS_MEM_SET(&stEquivPlmn, 0x00, sizeof(stEquivPlmn));

    pstNetWorkInfo = NAS_MML_GetNetworkInfo();

    if (NV_OK != NV_Read (en_NV_Item_EquivalentPlmn, &stEquivPlmn, sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadEquivalentPlmnNvim():WARNING: en_NV_Item_EquivalentPlmn Error");
        return;
    }

    if ( NVIM_MAX_EPLMN_NUM < stEquivPlmn.ucCount)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadEquivalentPlmnNvim():ERROR: EquivalentPlmn Num ERROR.");
        stEquivPlmn.ucCount = NVIM_MAX_EPLMN_NUM;
    }

    ulEquPlmnNum = stEquivPlmn.ucCount;

    if (ulEquPlmnNum > NAS_MML_MAX_EQUPLMN_NUM)
    {
        ulEquPlmnNum = NAS_MML_MAX_EQUPLMN_NUM;
    }

    for (i = 0; i < ulEquPlmnNum; i++)
    {
        pstNetWorkInfo->stEquPlmnInfo.astEquPlmnAddr[i].ulMcc =
                            (stEquivPlmn.struPlmnList[i].ucMcc[0]
                            | ((VOS_UINT32)stEquivPlmn.struPlmnList[i].ucMcc[1] << 8)
                            | ((VOS_UINT32)stEquivPlmn.struPlmnList[i].ucMcc[2] << 16));

        pstNetWorkInfo->stEquPlmnInfo.astEquPlmnAddr[i].ulMnc =
                           (stEquivPlmn.struPlmnList[i].ucMnc[0]
                           | ((VOS_UINT32)stEquivPlmn.struPlmnList[i].ucMnc[1] << 8)
                           | ((VOS_UINT32)stEquivPlmn.struPlmnList[i].ucMnc[2] << 16));
    }

    /* 从EPLMN列表中删除无效的网络 */
    ulEquPlmnNum = NAS_MML_DelInvalidPlmnFromList(ulEquPlmnNum, pstNetWorkInfo->stEquPlmnInfo.astEquPlmnAddr);
    pstNetWorkInfo->stEquPlmnInfo.ucEquPlmnNum = (VOS_UINT8)ulEquPlmnNum;

    return;
}
VOS_VOID NAS_MMC_ReadMaxForbRoamLaNvim(VOS_VOID)
{
    NAS_MMC_NVIM_MAX_FORB_ROAM_LA_STRU   stMaxForbRoamLa;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU *pstMiscellaneousCfgInfo = VOS_NULL_PTR;


    stMaxForbRoamLa.ucMaxForbRoamLaFlg = NAS_MMC_NV_ITEM_DEACTIVE;
    stMaxForbRoamLa.ucMaxForbRoamLaNum = 0;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    if (NV_OK != NV_Read(en_NV_Item_Max_Forb_Roam_La,
                         &stMaxForbRoamLa, sizeof(NAS_MMC_NVIM_MAX_FORB_ROAM_LA_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
                        "NAS_MMC_ReadMaxForbRoamLaNvim():WARNING: en_NV_Item_Max_Forb_Roam_La Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stMaxForbRoamLa.ucMaxForbRoamLaFlg)
    {
        pstMiscellaneousCfgInfo->ucMaxForbRoamLaFlg = stMaxForbRoamLa.ucMaxForbRoamLaFlg;
        pstMiscellaneousCfgInfo->ucMaxForbRoamLaNum = stMaxForbRoamLa.ucMaxForbRoamLaNum;
    }

    return;
}


VOS_VOID NAS_MMC_ReadGsmForbidAccessInfoNvim(VOS_VOID)
{
    NAS_MMC_NVIM_EFUST_SERVICE_CFG_STRU  stEfustServiceCfg;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU *pstMiscellaneousCfgInfo = VOS_NULL_PTR;


    PS_MEM_SET(&stEfustServiceCfg, 0x00, sizeof(NAS_MMC_NVIM_EFUST_SERVICE_CFG_STRU));

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    if (NV_OK != NV_Read(en_NV_Item_EFust_Service_Cfg,
                         &stEfustServiceCfg, sizeof(NAS_MMC_NVIM_EFUST_SERVICE_CFG_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
                        "NAS_MMC_ReadGsmForbidAccessInfoNvim:WARNING: en_NV_Item_EFust_Service_Cfg Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stEfustServiceCfg.ucStatus)
    {
        pstMiscellaneousCfgInfo->ucNvGsmForbidFlg = stEfustServiceCfg.ucForbidReg2GNetWork;
    }

    return;
}


VOS_VOID NAS_MMC_ReadRoamBrokerRegisterFailCntNvim(VOS_VOID)
{
    NAS_MMC_NVIM_REG_FAIL_CNT_STRU       stRegisterFailCnt;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU *pstMiscellaneousCfgInfo = VOS_NULL_PTR;


    stRegisterFailCnt.ucNvActiveFlag = VOS_FALSE;
    stRegisterFailCnt.ucRegFailCnt   = 0;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    if (NV_OK != NV_Read(en_NV_Item_Register_Fail_Cnt,
                         &stRegisterFailCnt, sizeof(NAS_MMC_NVIM_REG_FAIL_CNT_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRoamBrokerRegisterFailCntNvim():WARNING: en_Nv_Item_Register_Fail_Cnt Error");
        return;
    }

    pstMiscellaneousCfgInfo->ucRoamBrokerRegisterFailCnt = NAS_MML_MAX_CS_REG_FAIL_CNT;

    if ((VOS_TRUE == stRegisterFailCnt.ucNvActiveFlag)
     && (stRegisterFailCnt.ucRegFailCnt < NAS_MML_MAX_CS_REG_FAIL_CNT))
    {
        pstMiscellaneousCfgInfo->ucRoamBrokerRegisterFailCnt = stRegisterFailCnt.ucRegFailCnt;

        pstMiscellaneousCfgInfo->ucRoamBrokerActiveFlag      = VOS_TRUE;
    }


    return;
}


VOS_VOID NAS_MMC_ReadPsLociNvim(VOS_VOID)
{
    NAS_NVIM_PS_LOCI_SIM_FILES_STRU stPsLocInfo;


    PS_MEM_SET(&stPsLocInfo, 0x00, sizeof(stPsLocInfo));

    if (NV_OK != NV_Read(en_NV_Item_Ps_Loci,
                         &stPsLocInfo, NAS_NVIM_PS_LOCI_SIM_FILE_LEN))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPsLociNvim():WARNING: en_NV_Item_Ps_Loci Error");

        return;
    }

    /* 更新PTMSI的内容 */
    NAS_MMC_UpdatePtmsiFormPsLociFile(stPsLocInfo.aucPsLocInfo);

    /* 更新PTMSI SIGNATURE的内容 */
    NAS_MMC_UpdatePtmsiSignatureFormPsLociFile(stPsLocInfo.aucPsLocInfo);

    /* 更新RAI的内容 */
    NAS_MMC_UpdateRaiFormPsLociFile(stPsLocInfo.aucPsLocInfo);

    NAS_MML_UpdateSimPsLociFileContent(stPsLocInfo.aucPsLocInfo);

    /* 更新GPRS的状态 */
    if (stPsLocInfo.aucPsLocInfo[NAS_MML_PS_LOCI_SIM_FILE_LEN - 1] > NAS_MML_ROUTING_UPDATE_STATUS_ROUTING_AREA_NOT_ALLOWED )
    {
        stPsLocInfo.aucPsLocInfo[NAS_MML_PS_LOCI_SIM_FILE_LEN - 1] = NAS_MML_ROUTING_UPDATE_STATUS_ROUTING_AREA_NOT_ALLOWED;
    }
    NAS_MML_SetPsUpdateStatus(stPsLocInfo.aucPsLocInfo[NAS_MML_PS_LOCI_SIM_FILE_LEN - 1]);

    return;
}
VOS_VOID NAS_MMC_WritePsLociNvim(
    VOS_UINT8                          *pstPsLocInfo,
    VOS_UINT32                          ulSize
)
{
    VOS_UINT32                          i;
    NAS_MML_SIM_MS_IDENTITY_STRU       *pstMsIdentity = VOS_NULL_PTR;

    pstMsIdentity       = NAS_MML_GetSimMsIdentity();

    /* 非法PS LOC信息或长度 */
    if ((VOS_NULL_PTR == pstPsLocInfo)
     || (0 ==ulSize)
     || (ulSize > NAS_MML_PS_LOCI_SIM_FILE_LEN))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_WritePsLociNvim:ERROR: Invalid PS loc info!");

        return;
    }

    if (NV_OK != NV_Write(en_NV_Item_Ps_Loci, pstPsLocInfo, ulSize))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_WritePsLociNvim:ERROR: NV_Write Error");

        return;
    }

    /* 设置PTMSI的内容无效 */
    for (i = 0; i < NAS_MML_MAX_PTMSI_LEN; i++)
    {
        pstMsIdentity->aucPtmsi[i]  = pstPsLocInfo[i];
    }

    /* 设置PTMSI SIGNATURE的内容无效 */
    for (i = 0; i < NAS_MML_MAX_PTMSI_SIGNATURE_LEN; i++)
    {
        pstMsIdentity->aucPtmsiSignature[i] = pstPsLocInfo[i + NAS_MML_PTMSI_SIGNATRUE_POS_EFPSLOCI_FILE];
    }

    /* 更新RAI的内容 */
    NAS_MMC_UpdateRaiFormPsLociFile(pstPsLocInfo);

    /* 更新GPRS的状态 */
    NAS_MML_SetPsUpdateStatus(pstPsLocInfo[NAS_MML_PS_LOCI_SIM_FILE_LEN - 1]);

    return;
}
VOS_VOID NAS_MMC_ReadRoamCfgInfoNvim(VOS_VOID)
{
    NAS_NVIM_ROAM_CFG_INFO_STRU         stRoamFeature;
    NAS_MML_ROAM_CFG_INFO_STRU         *pstRoamCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulNvLenght;


    stRoamFeature.ucRoamFeatureFlg = NAS_MMC_NV_ITEM_DEACTIVE;
    stRoamFeature.ucRoamCapability = NAS_MML_ROAM_BUTT;
    ulNvLenght                     = 0;

    pstRoamCfgInfo = NAS_MML_GetRoamCfg();

    NV_GetLength(en_NV_Item_Roam_Capa, &ulNvLenght);
    if (NV_OK != NV_Read(en_NV_Item_Roam_Capa,
                         &stRoamFeature, ulNvLenght))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadRoamFeatureNvim():WARNING: en_NV_Item_Roam_Capa Error");

        return;
    }

    if ((NAS_MMC_NV_ITEM_ACTIVE == stRoamFeature.ucRoamFeatureFlg)
     && (stRoamFeature.ucRoamCapability < NAS_MML_ROAM_BUTT))
    {
        pstRoamCfgInfo->ucRoamFeatureFlg = stRoamFeature.ucRoamFeatureFlg;
        pstRoamCfgInfo->enRoamCapability = stRoamFeature.ucRoamCapability;
    }

    return;
}


VOS_VOID NAS_MMC_ReadDtSingleDomainPlmnSearchNvim(VOS_VOID)
{
    NAS_MMC_NVIM_SINGLE_DOMAIN_FAIL_CNT_STRU                stSingleDomainFailCnt;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU                    *pstMiscellaneousCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength = 0;


    stSingleDomainFailCnt.ucSingleDomainFailPlmnSrchFlag = NAS_MMC_NV_ITEM_DEACTIVE;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    NV_GetLength(en_NV_Item_SINGLE_DOMAIN_FAIL_SRCH_PLMN_CNT, &ulLength);
    if (NV_OK != NV_Read(en_NV_Item_SINGLE_DOMAIN_FAIL_SRCH_PLMN_CNT,
                         &stSingleDomainFailCnt.ucSingleDomainFailPlmnSrchFlag, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadDtSingleDomainPlmnSearchNvim():WARNING: en_NV_Item_SINGLE_DOMAIN_FAIL_SRCH_PLMN_CNT Error");

        return;
    }


    if ((NAS_MMC_NV_ITEM_ACTIVE != stSingleDomainFailCnt.ucSingleDomainFailPlmnSrchFlag)
     && (NAS_MMC_NV_ITEM_DEACTIVE != stSingleDomainFailCnt.ucSingleDomainFailPlmnSrchFlag))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadDtSingleDomainPlmnSearchNvim():WARNING: NV parameter Error");
        return;
    }

    pstMiscellaneousCfgInfo->ucSingleDomainFailPlmnSrchFlag
             = stSingleDomainFailCnt.ucSingleDomainFailPlmnSrchFlag;

}

VOS_VOID  NAS_MMC_ReadLastImsiNvim( VOS_VOID )
{

    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfg;
    NAS_MMC_NVIM_LAST_IMSI_STRU         stLastImsi;
    VOS_UINT32                          ulLength;

    ulLength = 0;


    PS_MEM_SET(&stLastImsi, 0x00, sizeof(NAS_MMC_NVIM_LAST_IMSI_STRU));

    pstRplmnCfg = NAS_MML_GetRplmnCfg();

    NV_GetLength(en_NV_Item_Last_Imsi, &ulLength);

    /* 保存在NVIM中,NVIM中保持的第一位是有效位，第二位以后才是IMSI内容 */
    if (NV_OK != NV_Read(en_NV_Item_Last_Imsi,
                    &stLastImsi, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
            "NAS_MMC_ReadLastImsiNvim(): WARNING: en_NV_Item_Last_Imsi Error");

        return;
    }

    PS_MEM_CPY(pstRplmnCfg->aucLastImsi,stLastImsi.aucImsi,NAS_MML_MAX_IMSI_LEN);


    return;
}

VOS_VOID NAS_MMC_UpdateLastImsi( VOS_VOID )
{

    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT8                           aucLastImsi[NAS_MML_MAX_IMSI_LEN];
    NAS_MMC_NVIM_LAST_IMSI_STRU         stLastImsi;
    VOS_UINT32                          ulLength;

    ulLength = 0;
    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    PS_MEM_SET(aucLastImsi, 0x00, NAS_MML_MAX_IMSI_LEN);
    PS_MEM_SET(&stLastImsi, 0x00, sizeof(NAS_MMC_NVIM_LAST_IMSI_STRU));

    if ( USIMM_API_SUCCESS != USIMM_GetCardIMSI(aucLastImsi) )
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_UpdateLastImsi:Get Imsi Failed");
        return;
    }

    PS_MEM_CPY(pstRplmnCfgInfo->aucLastImsi, aucLastImsi, sizeof(aucLastImsi));
    PS_MEM_CPY(stLastImsi.aucImsi, aucLastImsi, sizeof(aucLastImsi));

    NV_GetLength(en_NV_Item_Last_Imsi, &ulLength);

    /* 保存在NVIM中,NVIM中保持的第一位是有效位，第二位以后才是IMSI内容 */
    if (NV_OK != NV_Write (en_NV_Item_Last_Imsi,
                           &stLastImsi,
                           ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_UpdateLastImsi:Write NV Failed");
    }

}


VOS_VOID NAS_MMC_ReaducWcdmaPriorityGsmNvim(VOS_VOID)
{
    NAS_MMC_NVIM_WCDMA_PRIORITY_GSM_FLG_STRU                stWcdmaPriorityGsm;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU                    *pstMiscellaneousCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength = 0;

    stWcdmaPriorityGsm.ucWcdmaPriorityGsmFlg = NAS_MMC_NV_ITEM_DEACTIVE;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    /* 默认设置为不激活 */
    pstMiscellaneousCfgInfo->ucWcdmaPriorityGsmFlg             = NAS_MMC_NV_ITEM_DEACTIVE;
    pstMiscellaneousCfgInfo->ucSortAvailalePlmnListRatPrioFlg  = NAS_MMC_NV_ITEM_DEACTIVE;

    NV_GetLength(en_NV_Item_NVIM_WCDMA_PRIORITY_GSM_SUPPORT_FLG, &ulLength);
    if (NV_OK != NV_Read(en_NV_Item_NVIM_WCDMA_PRIORITY_GSM_SUPPORT_FLG,
                         &stWcdmaPriorityGsm, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadDtSingleDomainPlmnSearchNvim():WARNING: en_NV_Item_NVIM_WCDMA_PRIORITY_GSM_SUPPORT_FLG Error");

        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stWcdmaPriorityGsm.ucWcdmaPriorityGsmFlg)
    {
        pstMiscellaneousCfgInfo->ucWcdmaPriorityGsmFlg = stWcdmaPriorityGsm.ucWcdmaPriorityGsmFlg;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stWcdmaPriorityGsm.ucSortAvailalePlmnListRatPrioFlg)
    {
        pstMiscellaneousCfgInfo->ucSortAvailalePlmnListRatPrioFlg = stWcdmaPriorityGsm.ucSortAvailalePlmnListRatPrioFlg;
    }

    return;
}
VOS_VOID NAS_MMC_ReadPsOnlyCsServiceSupportNvim(VOS_VOID)
{
    NAS_NVIM_PS_ONLY_CS_SERVICE_SUPPORT_FLG_STRU            stPsOnlyCsServiceSupportFlg;
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU                    *pstMiscellaneousCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulLength;

    ulLength = 0;


    stPsOnlyCsServiceSupportFlg.ucPsOnlyCsServiceSupportFlg = NAS_MMC_NV_ITEM_DEACTIVE;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    NV_GetLength(en_NV_Item_NVIM_PS_ONLY_CS_SERVICE_SUPPORT_FLG, &ulLength);
    if (ulLength > sizeof(stPsOnlyCsServiceSupportFlg))
    {
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_NVIM_PS_ONLY_CS_SERVICE_SUPPORT_FLG,
                         &stPsOnlyCsServiceSupportFlg, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadDtSingleDomainPlmnSearchNvim():WARNING: en_NV_Item_SINGLE_DOMAIN_FAIL_SRCH_PLMN_CNT Error");

        return;
    }


    if ((NAS_MMC_NV_ITEM_ACTIVE != stPsOnlyCsServiceSupportFlg.ucPsOnlyCsServiceSupportFlg)
     && (NAS_MMC_NV_ITEM_DEACTIVE != stPsOnlyCsServiceSupportFlg.ucPsOnlyCsServiceSupportFlg))
    {
        pstMiscellaneousCfgInfo->ucPsOnlyCsServiceSupportFlg = NAS_MMC_NV_ITEM_DEACTIVE;
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReaducWcdmaPriorityGsmNvim():WARNING: NV parameter Error");
        return;
    }

    pstMiscellaneousCfgInfo->ucPsOnlyCsServiceSupportFlg
             = stPsOnlyCsServiceSupportFlg.ucPsOnlyCsServiceSupportFlg;

}


VOS_VOID NAS_MMC_ReadAPPConfigSupportNvim(VOS_VOID)
{
    NAS_MML_MISCELLANEOUS_CFG_INFO_STRU                    *pstMiscellaneousCfgInfo = VOS_NULL_PTR;
    NAS_NVIM_SYSTEM_APP_CONFIG_STRU                     stSysAppConfig;
    VOS_UINT32                                              ulLength;


    stSysAppConfig.usSysAppConfigType = SYSTEM_APP_BUTT;

    pstMiscellaneousCfgInfo = NAS_MML_GetMiscellaneousCfgInfo();

    NV_GetLength(en_NV_Item_System_APP_Config, &ulLength);
    if (ulLength > sizeof(stSysAppConfig))
    {
        return;
    }
    if (NV_OK != NV_Read(en_NV_Item_System_APP_Config,
                         &stSysAppConfig, ulLength))
    {
        pstMiscellaneousCfgInfo->usAppConfigSupportFlg  = SYSTEM_APP_MP;
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadAPPConfigSupportNvim():WARNING: read en_NV_Item_System_APP_Config Error");

        return;
    }


    if ( SYSTEM_APP_ANDROID < stSysAppConfig.usSysAppConfigType )
    {
        pstMiscellaneousCfgInfo->usAppConfigSupportFlg  = SYSTEM_APP_MP;
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadAPPConfigSupportNvim():WARNING: en_NV_Item_System_APP_Config error");

    }
    else
    {
        pstMiscellaneousCfgInfo->usAppConfigSupportFlg = stSysAppConfig.usSysAppConfigType ;
    }
    return;
}



VOS_VOID NAS_MMC_ReadAisRoamingNvim( VOS_VOID  )
{
    NAS_MMC_NVIM_AIS_ROAMING_CFG_STRU   stNvimAisRoamingCfg;
    NAS_MML_AIS_ROAMING_CFG_STRU       *pstAisRoamingCfg;


    PS_MEM_SET(&stNvimAisRoamingCfg, 0x00, sizeof(stNvimAisRoamingCfg));

    pstAisRoamingCfg = NAS_MML_GetAisRoamingCfg();

    if (NV_OK != NV_Read(en_NV_Item_NVIM_AIS_ROAMING_CFG,
                         &stNvimAisRoamingCfg, sizeof(stNvimAisRoamingCfg)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadAisRoamingNvim():WARNING: read en_NV_Item_NVIM_AIS_ROAMING_CFG Error");

        return;
    }

    pstAisRoamingCfg->ucEnableFlg            = stNvimAisRoamingCfg.ucEnableFlg;
    pstAisRoamingCfg->enHighPrioRatType      = stNvimAisRoamingCfg.ucHighPrioRatType;
    pstAisRoamingCfg->stHighPrioPlmnId.ulMcc = stNvimAisRoamingCfg.stHighPrioPlmnId.ulMcc;
    pstAisRoamingCfg->stHighPrioPlmnId.ulMnc = stNvimAisRoamingCfg.stHighPrioPlmnId.ulMnc;
    pstAisRoamingCfg->stSimHPlmnId.ulMcc     = stNvimAisRoamingCfg.stSimHPlmnId.ulMcc;
    pstAisRoamingCfg->stSimHPlmnId.ulMnc     = stNvimAisRoamingCfg.stSimHPlmnId.ulMnc;
}


VOS_VOID NAS_MMC_ReadUserAutoReselCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_USER_AUTO_RESEL_CFG_STRU    stUserAutoReselCfg;
    VOS_UINT32                               ulLength;

    ulLength = 0;


    PS_MEM_SET(&stUserAutoReselCfg, 0x00, sizeof(NAS_MMC_NVIM_USER_AUTO_RESEL_CFG_STRU));

    NV_GetLength(en_NV_Item_User_Auto_Resel_Switch, &ulLength);


    if (NV_OK != NV_Read(en_NV_Item_User_Auto_Resel_Switch,
                         &stUserAutoReselCfg, ulLength))
    {
        stUserAutoReselCfg.ucAutoReselActiveFlg  = VOS_FALSE;
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadUserAutoReselCfgNvim():WARNING: read en_NV_Item_User_Auto_Resel_Switch Error");

    }

    if (stUserAutoReselCfg.ucAutoReselActiveFlg == VOS_TRUE)
    {
        NAS_MML_SetUserAutoReselActiveFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetUserAutoReselActiveFlg(VOS_FALSE);
    }

    return;

}


VOS_VOID NAS_MMC_ReadScanCtrlNvim(VOS_VOID)
{
    NVIM_SCAN_CTRL_STRU                 stNvScanCtrl;
    VOS_UINT32                          ulLength;

    ulLength = 0;


    NV_GetLength(en_NV_Item_Scan_Ctrl_Para, &ulLength);
    if (ulLength > sizeof(stNvScanCtrl))
    {
        return;
    }

    PS_MEM_SET(&stNvScanCtrl, 0x00, sizeof(NVIM_SCAN_CTRL_STRU));

    /* 读NV项en_NV_Item_Scan_Ctrl_Para，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_Scan_Ctrl_Para,
                         &stNvScanCtrl, ulLength))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadScanCtrlNvim():WARNING: read en_NV_Item_Scan_Ctrl_Para Error");

        return;
    }

    /* 未激活，直接返回 */
    if (NAS_MMC_NV_ITEM_ACTIVE != stNvScanCtrl.ucNvimActiveFlg)
    {
        return;
    }

    /* 搜索控制已激活，更新搜索控制标识为VOS_TRUE */
    NAS_MML_SetScanCtrlEnableFlg(VOS_TRUE);

    return;
}



VOS_VOID NAS_MMC_ReadHPlmnSearchRegardLessMccNvim(VOS_VOID)
{
    NAS_MMC_NVIM_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_STRU   stHplmnSearchRegardlessMcc;


    PS_MEM_SET(&stHplmnSearchRegardlessMcc, 0x00, sizeof(stHplmnSearchRegardlessMcc));

    /* 读取NV */
    if (NV_OK != NV_Read(en_NV_Item_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT,
                         &stHplmnSearchRegardlessMcc, sizeof(NAS_MMC_NVIM_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadHPlmnSearchRegardLessMccNvim(): en_NV_Item_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT Error");
        return;
    }

    /*  有效性检查 */
    if (NAS_MMC_NV_ITEM_ACTIVE != stHplmnSearchRegardlessMcc.ucNvimActiveFlg)
    {
        return;
    }

    if (stHplmnSearchRegardlessMcc.ucCustomMccNum > NAS_MML_BG_SEARCH_REGARDLESS_MCC_NUMBER)
    {
        return;
    }

    /* 赋值到全局变量 */
    NAS_MML_SetBgSearchRegardlessMccList(stHplmnSearchRegardlessMcc.aulCustommMccList, stHplmnSearchRegardlessMcc.ucCustomMccNum);

    return;
}
VOS_UINT32 NAS_MMC_ConvertNivmActionToMmlType(
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_ENUM_UINT8   enNvAction,
    NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_ENUM_UINT8       *penMmlAction
)
{
    switch(enNvAction)
    {
        case NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_PLMN_SELECTION:
            *penMmlAction = NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_PLMN_SELECTION;
            break;

        case NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_NORMAL_CAMP_ON:
            *penMmlAction = NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_NORMAL_CAMP_ON;
            break;

        case NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_OPTIONAL_PLMN_SELECTION:
            *penMmlAction = NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_OPTIONAL_PLMN_SELECTION;
            break;

        case NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_LIMITED_CAMP_ON:
            *penMmlAction = NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_LIMITED_CAMP_ON;
            break;

        case NAS_MMC_NVIM_SINGLE_DOMAIN_ROAMING_REG_FAIL_ACTION_PLMN_SELECTION:
            *penMmlAction = NAS_MML_SINGLE_DOMAIN_ROAMING_REG_FAIL_ACTION_PLMN_SELECTION;
            break;

        default:
            *penMmlAction = NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_BUTT;
            return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_VOID NAS_MMC_ReadPrioHplmnActCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_PRIO_HPLMNACT_CFG_STRU    stHplmnActCfg;
    NAS_MML_PRIO_HPLMNACT_CFG_STRU        *pstPrioHplmnActCfg = VOS_NULL_PTR;


    PS_MEM_SET(&stHplmnActCfg, 0x00, sizeof(stHplmnActCfg));

    pstPrioHplmnActCfg = NAS_MML_GetPrioHplmnActCfg();

    if (NV_OK != NV_Read(en_NV_Item_PRIO_HPLMNACT_CFG,
                        &stHplmnActCfg, sizeof(NAS_MMC_NVIM_PRIO_HPLMNACT_CFG_STRU)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,
           "NAS_MMC_ReadPrioHplmnActCfgNvim(): read en_NV_Item_PRIO_HPLMNACT_CFG fail");

        return;
    }

    /* 获取该定制是否激活 */
    if (NAS_MMC_NV_ITEM_ACTIVE == stHplmnActCfg.ucStatus)
    {
        pstPrioHplmnActCfg->ucActiveFlg = VOS_TRUE;
    }
    else
    {
        pstPrioHplmnActCfg->ucActiveFlg = VOS_FALSE;
    }

    /* 获取定制的优先的HPLMNAct,只考虑产品支持的接入技术 */
#if (FEATURE_ON == FEATURE_LTE)

    pstPrioHplmnActCfg->usPrioHplmnAct = (stHplmnActCfg.usPrioHplmnAct & NAS_MML_SIM_UE_SUPPORT_RAT_SUPPORT_LTE);

#else
    pstPrioHplmnActCfg->usPrioHplmnAct = (stHplmnActCfg.usPrioHplmnAct & NAS_MML_SIM_UE_SUPPORT_RAT);

#endif

    return;
}


VOS_UINT32 NAS_MMC_ConvertNivmRegCauseToMmlType(
    NAS_MMC_NVIM_REG_FAIL_CAUSE_ENUM_UINT16                 enNvCause,
    NAS_MML_REG_FAIL_CAUSE_ENUM_UINT16                     *penMmlCause
)
{
    switch (enNvCause)
    {
        case NAS_MMC_NVIM_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW_IN_PLMN:
            *penMmlCause = NAS_MML_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW_IN_PLMN;
            break;

        case NAS_MMC_NVIM_REG_FAIL_CAUSE_TIMER_TIMEOUT:
            *penMmlCause = NAS_MML_REG_FAIL_CAUSE_TIMER_TIMEOUT;
            break;

        default:
            *penMmlCause = NAS_MML_REG_FAIL_CAUSE_BUTT;
            return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_VOID NAS_MMC_ReadSingleDomainFailActionListNvim(VOS_VOID)
{
    NAS_MMC_NVIM_SINGLE_DOMAIN_FAIL_ACTION_LIST_STRU    stSingleDomainFailAction;
    NAS_MML_SINGLE_DOMAIN_FAIL_ACTION_LIST_CTX_STRU    *pstSingleDomainFailActionCtx = VOS_NULL_PTR;
    VOS_UINT32                                          i;
    NAS_MML_REG_FAIL_CAUSE_ENUM_UINT16                  enRegCause;
    NAS_MML_SINGLE_DOMAIN_REG_FAIL_ACTION_ENUM_UINT8    enAction;


    PS_MEM_SET(&stSingleDomainFailAction, 0x00, sizeof(stSingleDomainFailAction));

    /* 读取NV */
    if (NV_OK != NV_Read(en_NV_Item_SINGLE_DOMAIN_FAIL_ACTION_LIST,
                         &stSingleDomainFailAction, sizeof(NAS_MMC_NVIM_SINGLE_DOMAIN_FAIL_ACTION_LIST_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSingleDomainFailActionListNvim(): en_NV_Item_SINGLE_DOMAIN_FAIL_ACTION_LIST Error");
        return;
    }

    /* 有效性检查 */
    if (NAS_MMC_NV_ITEM_ACTIVE != stSingleDomainFailAction.ucActiveFlag)
    {
        return;
    }

    /* 个数非法 */
    if (stSingleDomainFailAction.ucCount > NAS_MML_SINGLE_DOMAIN_FAIL_ACTION_MAX_LIST)
    {
        return;
    }

    pstSingleDomainFailActionCtx = NAS_MML_GetSingleDomainFailActionCtx();

    pstSingleDomainFailActionCtx->ucActiveFlag = VOS_TRUE;

    /* 将NV项的值保存到全局变量，需要将NV中定制的ACTION值转换为MMC定义的ACTION值 */
    for (i = 0; i < stSingleDomainFailAction.ucCount; i++)
    {
        if (NAS_MMC_NVIM_REG_DOMAIN_CS == stSingleDomainFailAction.astSingleDomainFailActionList[i].enDomain)
        {
            pstSingleDomainFailActionCtx->astSingleDomainFailActionList[pstSingleDomainFailActionCtx->ucCount].enDomain
                = NAS_MML_REG_DOMAIN_CS;
        }
        else if (NAS_MMC_NVIM_REG_DOMAIN_PS == stSingleDomainFailAction.astSingleDomainFailActionList[i].enDomain)
        {
            pstSingleDomainFailActionCtx->astSingleDomainFailActionList[pstSingleDomainFailActionCtx->ucCount].enDomain
                = NAS_MML_REG_DOMAIN_PS;
        }
        else
        {
            continue;
        }

        if (VOS_FALSE == NAS_MMC_ConvertNivmRegCauseToMmlType(stSingleDomainFailAction.astSingleDomainFailActionList[i].enRegCause, &enRegCause))
        {
            continue;
        }

        if (VOS_FALSE == NAS_MMC_ConvertNivmActionToMmlType(stSingleDomainFailAction.astSingleDomainFailActionList[i].enAction, &enAction))
        {
            continue;
        }


        pstSingleDomainFailActionCtx->astSingleDomainFailActionList[pstSingleDomainFailActionCtx->ucCount].enRegCause
            = enRegCause;

        pstSingleDomainFailActionCtx->astSingleDomainFailActionList[pstSingleDomainFailActionCtx->ucCount].enAction
            = enAction;

        pstSingleDomainFailActionCtx->ucCount++;
    }

    return;
}


VOS_VOID NAS_MMC_ReadRegFailNetWorkFailureCustomFlagNvim(VOS_VOID)
{
    NAS_MMC_NVIM_REG_FAIL_NETWORK_FAILURE_CUSTOM_FLG_STRU   stCsRegFailNetFailureCustom;
    VOS_UINT32                                              ulLength;


    PS_MEM_SET(&stCsRegFailNetFailureCustom, 0x00, sizeof(NAS_MMC_NVIM_REG_FAIL_NETWORK_FAILURE_CUSTOM_FLG_STRU));

    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_CS_FAIL_NETWORK_FAILURE_PLMN_SEARCH_FLAG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_REG_FAIL_NETWORK_FAILURE_CUSTOM_FLG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRegFailNetWorkFailureCustomFlagNvim():WARNING: en_NV_Item_CS_FAIL_NETWORK_FAILURE_PLMN_SEARCH_FLAG length Error");
        return;
    }

    /* 读取NV */
    if (NV_OK != NV_Read(en_NV_Item_CS_FAIL_NETWORK_FAILURE_PLMN_SEARCH_FLAG,
                         &stCsRegFailNetFailureCustom, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadRegFailNetWorkFailureCustomFlagNvim(): en_NV_Item_CS_FAIL_NETWORK_FAILURE_PLMN_SEARCH_FLAG Error");
        return;
    }

    /* NV有效性检查 */
    if (NAS_MMC_NV_ITEM_ACTIVE != stCsRegFailNetFailureCustom.ucNvimActiveFlg)
    {
        return;
    }

    /* 赋值到全局变量 */
    NAS_MML_SetRegFailNetWorkFailureCustomFlag(VOS_TRUE);

    return;
}


VOS_VOID NAS_MMC_ReadPlmnExactlyCompareNvim(VOS_VOID)
{
    NAS_MMC_NVIM_PLMN_EXACTLY_COMPARE_FLAG_STRU             stPlmnExactlyCompara;
    VOS_UINT32                                              ulLength;


    PS_MEM_SET(&stPlmnExactlyCompara, 0x00, sizeof(NAS_MMC_NVIM_PLMN_EXACTLY_COMPARE_FLAG_STRU));

    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_PLMN_EXACTLY_COMPARE_FLG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_PLMN_EXACTLY_COMPARE_FLAG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlmnExactlyCompareNvim():WARNING: en_NV_Item_PLMN_EXACTLY_COMPARE_FLG length Error");
        return;
    }

    /* 读取NV */
    if (NV_OK != NV_Read (en_NV_Item_PLMN_EXACTLY_COMPARE_FLG, &stPlmnExactlyCompara, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlmnExactlyCompareNvim(): en_NV_Item_PLMN_EXACTLY_COMPARE_FLG Error");
        return;
    }

    /* NV有效性检查 */
    if (VOS_TRUE == stPlmnExactlyCompara.ucPlmnExactlyCompareFlag)
    {
        /* 赋值到全局变量 */
        NAS_MML_SetPlmnExactlyComparaFlg(stPlmnExactlyCompara.ucPlmnExactlyCompareFlag);
    }

    return;
}

VOS_VOID NAS_MMC_ReadCustomizeServiceNvim(VOS_VOID)
{
    NAS_NVIM_CUSTOMIZE_SERVICE_STRU                         stCustomServ;
    VOS_UINT32                                              ulLength;


    ulLength = 0;

    PS_MEM_SET(&stCustomServ, (VOS_CHAR)0x0, sizeof(NAS_NVIM_CUSTOMIZE_SERVICE_STRU));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_CustomizeService, &ulLength);

    if (ulLength > sizeof(NAS_NVIM_CUSTOMIZE_SERVICE_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCustomizeServiceNvim():WARNING: en_NV_Item_CustomizeService length Error");
        return;
    }

    /* 读取en_NV_Item_CustomizeService NV */
    if (NV_OK != NV_Read (en_NV_Item_CustomizeService, &stCustomServ, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCustomizeServiceNvim(): en_NV_Item_CustomizeService Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_DEACTIVE == stCustomServ.ulStatus)
    {
        NAS_MML_SetSupportCsServiceFLg(VOS_FALSE);
    }
    
    if ((NAS_MMC_NV_ITEM_ACTIVE == stCustomServ.ulStatus)
     && (VOS_FALSE == stCustomServ.ulCustomizeService))
    {
        NAS_MML_SetSupportCsServiceFLg(VOS_FALSE);
    }

    return;

}




VOS_VOID NAS_MMC_ReadHplmnRegisterCtrlNvim(VOS_VOID)
{
    NAS_MMC_NVIM_HPLMN_REGISTER_CTRL_FLAG_STRU              stHplmnRegisterCtrlFlg;
    VOS_UINT32                                              ulLength;


    PS_MEM_SET(&stHplmnRegisterCtrlFlg, 0x00, sizeof(NAS_MMC_NVIM_HPLMN_REGISTER_CTRL_FLAG_STRU));

    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Hplmn_Register_Ctrl_Flg, &ulLength);

    if (ulLength > sizeof(stHplmnRegisterCtrlFlg))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadHplmnRegisterCtrlNvim():WARNING: en_NV_Item_Hplmn_Register_Ctrl_Flg length Error");
        return;
    }

    /* 读取NV */
    if (NV_OK != NV_Read (en_NV_Item_Hplmn_Register_Ctrl_Flg, &stHplmnRegisterCtrlFlg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadHplmnRegisterCtrlNvim(): en_NV_Item_Hplmn_Register_Ctrl_Flg Error");
        return;
    }

    /* NV有效性检查 */
    if (VOS_TRUE == stHplmnRegisterCtrlFlg.ucHplmnRegisterCtrlFlg)
    {
        /* 赋值到全局变量 */
        NAS_MML_SetHplmnRegisterCtrlFlg(stHplmnRegisterCtrlFlg.ucHplmnRegisterCtrlFlg);
    }

    return;
}


VOS_VOID NAS_MMC_ReadH3gCtrlNvim(VOS_VOID)
{
    NAS_MMC_NVIM_H3G_CTRL_FLAG_STRU     stH3gCtrlFlg;
    VOS_UINT32                          ulLength;

    PS_MEM_SET(&stH3gCtrlFlg, 0x00, sizeof(NAS_MMC_NVIM_H3G_CTRL_FLAG_STRU));

    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_H3g_Customization, &ulLength);

    if (ulLength > sizeof(stH3gCtrlFlg))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadH3gCtrlNvim():WARNING: en_NV_Item_H3g_Customization length Error");
        return;
    }

    /* 读取NV */
    if (NV_OK != NV_Read (en_NV_Item_H3g_Customization, &stH3gCtrlFlg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadH3gCtrlNvim(): en_NV_Item_H3g_Customization Error");
        return;
    }

    /* 赋值到全局变量 */
    NAS_MML_SetH3gCtrlFlg(stH3gCtrlFlg.ucH3gCtrlFlg);

    return;
}

VOS_VOID NAS_MMC_ReadCellSignReportCfgNvim( VOS_VOID )
{

    NAS_MMC_MAINTAIN_CTX_STRU                              *pstMaintainInfo = VOS_NULL_PTR;
    NAS_NVIM_CELL_SIGN_REPORT_CFG_STRU                      stCellSignReportCfg;
    VOS_UINT32                                              ulLength;

    pstMaintainInfo = NAS_MMC_GetMaintainInfo();
    PS_MEM_SET(&stCellSignReportCfg, (VOS_CHAR)0x0, sizeof(stCellSignReportCfg));


    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_CELL_SIGN_REPORT_CFG, &ulLength);

    if (ulLength > sizeof(NAS_NVIM_CELL_SIGN_REPORT_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCellSignReportCfgNvim():WARNING: en_NV_Item_CELL_SIGN_REPORT_CFG length Error");
        return;
    }

    if(NV_OK != NV_Read(en_NV_Item_CELL_SIGN_REPORT_CFG,
                        &stCellSignReportCfg,
                        ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCellSignReportCfgNvim:read en_NV_Item_CELL_SIGN_REPORT_CFG failed");
        return;
    }

    /* 获取nv项中信号改变门限配置，开机时需通知接入层 */
    pstMaintainInfo->ucSignThreshold = stCellSignReportCfg.ucSignThreshold;

    if (stCellSignReportCfg.ucSignThreshold > NAS_MMC_MAX_SIGN_THRESHOLD)
    {
        pstMaintainInfo->ucSignThreshold = 0;
    }

    /* 获取nv项中信号改变间隔时间，开机时需通知接入层 */
    pstMaintainInfo->ucMinRptTimerInterval = stCellSignReportCfg.ucMinRptTimerInterval;

    if (stCellSignReportCfg.ucMinRptTimerInterval > NAS_MMC_MAX_SIGN_TIMERINTERVAL)
    {
        pstMaintainInfo->ucMinRptTimerInterval = 0;
    }

    return;
}
VOS_VOID NAS_MMC_WriteCellSignReportCfgNvim(VOS_UINT8 ucSignThreshold,VOS_UINT8 ucMinRptTimerInterval)
{
    NAS_NVIM_CELL_SIGN_REPORT_CFG_STRU  stCellSignReportCfg;
    VOS_UINT32                          ulLength;

    PS_MEM_SET(&stCellSignReportCfg, 0x0, sizeof(stCellSignReportCfg));
    ulLength = 0;

    stCellSignReportCfg.ucSignThreshold       = ucSignThreshold;
    stCellSignReportCfg.ucMinRptTimerInterval = ucMinRptTimerInterval;

    NV_GetLength(en_NV_Item_CELL_SIGN_REPORT_CFG, &ulLength);
    if (NV_OK != NV_Write(en_NV_Item_CELL_SIGN_REPORT_CFG,
                          &stCellSignReportCfg,
                          ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_WriteCellSignReportCfgNvim():en_NV_Item_CELL_SIGN_REPORT_CFG Error");
    }

    return;
}


VOS_VOID NAS_MMC_ReadPlatformRatCapNvim( VOS_VOID )
{
    NAS_MML_PLATFORM_RAT_CAP_STRU              *pstPlatformRatCap = VOS_NULL_PTR;
    PLATAFORM_RAT_CAPABILITY_STRU               stNvPlatformRatCap;
    NAS_MML_PLATFORM_RAT_TYPE_ENUM_UINT8        enNasPlatformRatCap;
    VOS_UINT32                                  ulLength;
    VOS_UINT8                                   ucIndex;
    VOS_UINT32                                  i;

    ulLength = 0;
    pstPlatformRatCap = NAS_MML_GetPlatformRatCap();
    PS_MEM_SET(&stNvPlatformRatCap, (VOS_CHAR)0x0, sizeof(stNvPlatformRatCap));

    /* 初始化为GSM ONLY */
    pstPlatformRatCap->ucRatNum      = NAS_MML_MIN_PLATFORM_RAT_NUM;
    pstPlatformRatCap->aenRatPrio[0] = NAS_MML_PLATFORM_RAT_TYPE_GSM;

    /* 先获取NV的长度 */
    ulLength = 0;
    NV_GetLength(en_NV_Item_Platform_RAT_CAP, &ulLength);

    if (ulLength > sizeof(PLATAFORM_RAT_CAPABILITY_STRU))
    {
        NAS_MMC_SndOmPlatformRatCap();

        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlatformRatCapNvim():WARNING: en_NV_Item_PLATFORM_RAT_CAP length Error");

        NAS_TRACE_HIGH("en_NV_Item_PLATFORM_RAT_CAP length Error, ulLength = %d", ulLength);
        NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_READ_PLATFORM_RAT_CAP_NVIM_FAILURE);

        return;
    }

    /* 读NV项en_NV_Item_PLATFORM_RAT_CAP，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_Platform_RAT_CAP,
                         &stNvPlatformRatCap, sizeof(PLATAFORM_RAT_CAPABILITY_STRU)))
    {
        NAS_MMC_SndOmPlatformRatCap();

        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlatformRatCapNvim():WARNING: read en_NV_Item_PLATFORM_RAT_CAP Error");

        NAS_TRACE_HIGH("read en_NV_Item_PLATFORM_RAT_CAP Error");
        NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_READ_PLATFORM_RAT_CAP_NVIM_FAILURE);

        return;
    }

    /* 有效性检查：无效，直接返回 */
    if (VOS_FALSE == NAS_MMC_IsPlatformRatCapNvimValid(&stNvPlatformRatCap))
    {
        NAS_MMC_SndOmPlatformRatCap();

        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadPlatformRatCapNvim():WARNING: en_NV_Item_PLATFORM_RAT_CAP Invalid");

        NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_READ_PLATFORM_RAT_CAP_NVIM_FAILURE);

        return;
    }

    /* 赋值到全局变量 */
    ucIndex = 0;
    for (i = 0; i < stNvPlatformRatCap.usRatNum; i++)
    {
        if (VOS_TRUE == NAS_MMC_ConvertNvimTypePlatformRatCapToNasType((stNvPlatformRatCap.aenRatList[i]),
                                                                        &enNasPlatformRatCap))
        {
            pstPlatformRatCap->aenRatPrio[ucIndex] = enNasPlatformRatCap;
            ucIndex++;
        }
    }

    if (ucIndex > 0)
    {
        pstPlatformRatCap->ucRatNum = ucIndex;
    }

    /* 可维可测，打印LOG */
    NAS_MMC_SndOmPlatformRatCap();
    
    NAS_TRACE_HIGH("read en_NV_Item_PLATFORM_RAT_CAP correct, RatNum = %d, aenRatList[0]= %d, aenRatList[1]= %d, aenRatList[2]= %d, aenRatList[3]= %d, aenRatList[4]= %d,aenRatList[5]= %d, aenRatList[6]= %d",
                    stNvPlatformRatCap.usRatNum, stNvPlatformRatCap.aenRatList[0], stNvPlatformRatCap.aenRatList[1], stNvPlatformRatCap.aenRatList[2], stNvPlatformRatCap.aenRatList[3], stNvPlatformRatCap.aenRatList[4], stNvPlatformRatCap.aenRatList[5], stNvPlatformRatCap.aenRatList[6]);

    return;

}


VOS_UINT32 NAS_MMC_IsPlatformRatCapNvimValid(
    PLATAFORM_RAT_CAPABILITY_STRU              *pstNvPlatformRatCap
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          j;

    /* 如果平台支持的接入技术个数为0，返回无效 */
    if (0 == pstNvPlatformRatCap->usRatNum || pstNvPlatformRatCap->usRatNum > PLATFORM_MAX_RAT_NUM)
    {
        NAS_TRACE_HIGH("en_NV_Item_PLATFORM_RAT_CAP RatNum Invalid, RatNum = %d", pstNvPlatformRatCap->usRatNum);

        return VOS_FALSE;
    }

    for (i = 0; i < pstNvPlatformRatCap->usRatNum; i++)
    {
        /* 如果平台支持的接入技术有重复，返回无效  */
        for (j = i + 1 ; j < pstNvPlatformRatCap->usRatNum; j++ )
        {
            if (pstNvPlatformRatCap->aenRatList[i] == pstNvPlatformRatCap->aenRatList[j])
            {
                NAS_TRACE_HIGH("en_NV_Item_PLATFORM_RAT_CAP Rat duplicate, RatNum = %d, aenRatList[%d] = aenRatList[%d] = %d", pstNvPlatformRatCap->usRatNum, i, j, pstNvPlatformRatCap->aenRatList[i]);

                return VOS_FALSE;
            }
        }

        /* 如果平台支持的接入技术无效，返回无效 */
        if (PLATFORM_RAT_BUTT <= pstNvPlatformRatCap->aenRatList[i])
        {
            NAS_TRACE_HIGH("en_NV_Item_PLATFORM_RAT_CAP Rat invalid, aenRatList[%d] = %d", i, pstNvPlatformRatCap->aenRatList[i]);

            return VOS_FALSE;
        }
    }

    return VOS_TRUE;

}
VOS_UINT32 NAS_MMC_ConvertNvimTypePlatformRatCapToNasType(
    PLATFORM_RAT_TYPE_ENUM_UINT16                       enNvPlatformRatCap,
    NAS_MML_PLATFORM_RAT_TYPE_ENUM_UINT8               *penNasPlatformRatCap
)
{
    VOS_UINT32                          ulRslt;
    ulRslt  = VOS_FALSE;

    switch (enNvPlatformRatCap)
    {
        case PLATFORM_RAT_GSM:
            *penNasPlatformRatCap = NAS_MML_PLATFORM_RAT_TYPE_GSM;
            ulRslt                = VOS_TRUE;
            break;

        case PLATFORM_RAT_WCDMA:
            *penNasPlatformRatCap = NAS_MML_PLATFORM_RAT_TYPE_WCDMA;
            ulRslt                = VOS_TRUE;
            break;

#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
        case PLATFORM_RAT_TDS:
            *penNasPlatformRatCap = NAS_MML_PLATFORM_RAT_TYPE_TD_SCDMA;
            ulRslt                = VOS_TRUE;
            break;
#endif

#if   (FEATURE_ON == FEATURE_LTE)
        case PLATFORM_RAT_LTE:
            *penNasPlatformRatCap = NAS_MML_PLATFORM_RAT_TYPE_LTE;
            ulRslt                = VOS_TRUE;
            break;
#endif
        default:
            ulRslt                = VOS_FALSE;
            break;
    }
    return ulRslt;

}


#if  (FEATURE_ON == FEATURE_LTE)

VOS_VOID NAS_MMC_ReadEnableLteTimerLenNvim(VOS_VOID)
{
    NAS_MMC_NVIM_ENABLE_LTE_TIMER_LEN_STRU                  stEnableLteTimerLen;
    VOS_UINT32                                              ulLength;

    PS_MEM_SET(&stEnableLteTimerLen, 0x00, sizeof(stEnableLteTimerLen));
    ulLength = 0;

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Enable_Lte_Timer_Len, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_ENABLE_LTE_TIMER_LEN_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadEnableLteTimerLenNvim(): en_NV_Item_Enable_Lte_Timer_Len length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_Enable_Lte_Timer_Len,
                         &stEnableLteTimerLen, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadEnableLteTimerLenNvim(): en_NV_Item_Enable_Lte_Timer_Len error");
        return;
    }

    /* nv项激活且设置enable lte定时器时长为0，则更新为默认值5分钟 */
    if (VOS_TRUE == stEnableLteTimerLen.ucNvimActiveFlag)
    {
        if (0 == stEnableLteTimerLen.ucCsfbEmgCallEnableLteTimerLen)
        {
            NAS_MML_SetCsfbEmgCallEnableLteTimerLen(NAS_MML_ENABLE_LTE_TIMER_DEFAULT_LEN);
        }
        else
        {
            /* 更新MML全局变量中的enable lte定时器时长,nv项单位是分钟，启定时器单位是毫秒 */
            NAS_MML_SetCsfbEmgCallEnableLteTimerLen(stEnableLteTimerLen.ucCsfbEmgCallEnableLteTimerLen * NAS_MML_SIXTY_SECOND
                                         * NAS_MML_ONE_THOUSAND_MILLISECOND);
        }

        if (0 == stEnableLteTimerLen.ucCsPsMode1EnableLteTimerLen)
        {
            NAS_MML_SetCsPsMode13GPPEnableLteTimerLen(NAS_MML_ENABLE_LTE_TIMER_DEFAULT_LEN);
        }
        else
        {
            NAS_MML_SetCsPsMode13GPPEnableLteTimerLen(stEnableLteTimerLen.ucCsPsMode1EnableLteTimerLen * NAS_MML_SIXTY_SECOND
                                         * NAS_MML_ONE_THOUSAND_MILLISECOND);
        }
    }

    return;
}

/* Added by c00318887 for 移植T3402 , 2015-6-17, begin */
/*****************************************************************************
 函 数 名  : NAS_MMC_ReadDisableLteStartT3402EnableLteCfgNvim
 功能描述  : 读取en_NV_Item_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG nv项内容
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年04月28日
    作    者   : c00318887
    修改内容   : for file refresh 新增
*****************************************************************************/
VOS_VOID NAS_MMC_ReadDisableLteStartT3402EnableLteCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG_STRU        stDisableLteStartT3402EnableLteCfg;
    VOS_UINT32                                                      ulLength;

    ulLength = 0;
    PS_MEM_SET(&stDisableLteStartT3402EnableLteCfg, 0x0, sizeof(NAS_MMC_NVIM_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG_STRU));

    /* 先获取NV的长度 */
    (VOS_VOID)NV_GetLength(en_NV_Item_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadDisableLteStartT3402EnableLteCfgNvim(): en_NV_Item_Enable_Lte_Timer_Len_Using_T3402_CFG length Error");
        return;
    }


    if (NV_OK != NV_Read (en_NV_Item_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG, &stDisableLteStartT3402EnableLteCfg, sizeof(NAS_MMC_NVIM_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadDisableLteStartT3402EnableLteCfgNvim():WARNING: en_NV_Item_Enable_Lte_Timer_Len_Using_T3402_CFG Error");
        return;
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stDisableLteStartT3402EnableLteCfg.ucT3402Flag)
    {
        NAS_MML_SetDisableLteStartT3402EnableLteFlag(stDisableLteStartT3402EnableLteCfg.ucT3402Flag);
    }

    if (NAS_MMC_NV_ITEM_ACTIVE == stDisableLteStartT3402EnableLteCfg.ucHighPrioRatTimerNotEnableLteFlag)
    {
        NAS_MML_SetHighPrioRatBgEnableLteFlag(NAS_MML_HIGH_PRIO_RAT_BG_SEARCH_PROC_LTE_NOT_ENABLE_LTE);
    }

    return;
}

/* Added by c00318887 for 移植T3402 , 2015-6-17, end */



VOS_VOID NAS_MMC_ReadCsfbRauFollowOnFlgNvim(VOS_VOID)
{
    NAS_MMC_CSFB_RAU_FOLLOW_ON_FLAG_STRU                    stCsfbRauFollowOnFlg;
    VOS_UINT32                                              ulLength;

    ulLength = 0;
    PS_MEM_SET(&stCsfbRauFollowOnFlg, 0x00, sizeof(stCsfbRauFollowOnFlg));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_CSFB_RAU_FOLLOW_ON_FLAG, &ulLength);

    if (ulLength > sizeof(NAS_MMC_CSFB_RAU_FOLLOW_ON_FLAG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCsfbRauFollowOnFlgNvim(): en_NV_Item_CSFB_RAU_FOLLOW_ON_FLAG length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_CSFB_RAU_FOLLOW_ON_FLAG,
                         &stCsfbRauFollowOnFlg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadCsfbRauFollowOnFlgNvim(): en_NV_Item_CSFB_RAU_FOLLOW_ON_FLAG error");
        return;
    }

    if (VOS_TRUE == stCsfbRauFollowOnFlg.ucIsRauNeedFollowOnCsfbMtFlg)
    {
         NAS_MML_SetIsRauNeedFollowOnCsfbMtFlg(VOS_TRUE);
    }

    if (VOS_TRUE == stCsfbRauFollowOnFlg.ucIsRauNeedFollowOnCsfbMoFlg)
    {
         NAS_MML_SetIsRauNeedFollowOnCsfbMoFlg(VOS_TRUE);
    }

    return;
}


VOS_VOID NAS_MMC_ReadLteCustomMccInfoNvim(VOS_VOID)
{
    LTE_CUSTOM_MCC_INFO_STRU            stLteCustomMccInfo;
    VOS_UINT32                          ulLength;

    ulLength = 0;
    PS_MEM_SET(&stLteCustomMccInfo, 0x00, sizeof(stLteCustomMccInfo));

    /* 先获取NV的长度 */
    NV_GetLength(EN_NV_ID_LTE_CUSTOM_MCC_INFO, &ulLength);

    if (ulLength > sizeof(LTE_CUSTOM_MCC_INFO_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteCustomMccInfoNvim(): EN_NV_ID_LTE_CUSTOM_MCC_INFO length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(EN_NV_ID_LTE_CUSTOM_MCC_INFO,
                         &stLteCustomMccInfo, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteCustomMccInfoNvim(): EN_NV_ID_LTE_CUSTOM_MCC_INFO error");
        return;
    }

    if (VOS_TRUE == stLteCustomMccInfo.ucSwitchFlag)
    {
         NAS_MML_SetIsDelFddBandSwitchOnFlg(VOS_TRUE);
    }

    return;
}


#endif


VOS_VOID NAS_MMC_ReadSvlteSupportFlagNvim(VOS_VOID)
{
#if (FEATURE_ON == FEATURE_MULTI_MODEM)
    SVLTE_SUPPORT_FLAG_STRU             stSvlteSupportFlag;
    VOS_UINT32                          ulLength;

    ulLength = 0;
    PS_MEM_SET(&stSvlteSupportFlag, 0x00, sizeof(stSvlteSupportFlag));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_SVLTE_FLAG, &ulLength);

    if (ulLength > sizeof(SVLTE_SUPPORT_FLAG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSvlteSupportFlagNvim(): en_NV_Item_SVLTE_FLAG length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_SVLTE_FLAG,
                         &stSvlteSupportFlag, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSvlteSupportFlagNvim(): en_NV_Item_SVLTE_FLAG error");
        return;
    }

    if (VOS_TRUE == stSvlteSupportFlag.ucSvlteSupportFlag)
    {
         NAS_MML_SetSvlteSupportFlag(VOS_TRUE);
    }
#endif

    return;
}




VOS_VOID NAS_MMC_ReadDsdaPlmnSearchEnhancedCfgNvim(VOS_VOID)
{
#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    NV_DSDA_PLMN_SEARCH_ENHANCED_CFG_STRU                   stDsdaPlmnSearchEnhancedCfg;
    VOS_UINT32                                              ulLength;

    ulLength = 0;
    PS_MEM_SET(&stDsdaPlmnSearchEnhancedCfg, 0x00, sizeof(stDsdaPlmnSearchEnhancedCfg));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_DSDA_PLMN_SEARCH_ENHANCED_CFG, &ulLength);

    if (ulLength > sizeof(NV_DSDA_PLMN_SEARCH_ENHANCED_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDsdaPlmnSearchEnhancedCfgNvim(): en_NV_Item_DSDA_PLMN_SEARCH_ENHANCED_CFG length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_DSDA_PLMN_SEARCH_ENHANCED_CFG,
                         &stDsdaPlmnSearchEnhancedCfg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDsdaPlmnSearchEnhancedCfgNvim(): en_NV_Item_DSDA_PLMN_SEARCH_ENHANCED_CFG error");
        return;
    }

    if (NAS_MMC_UTRAN_SKIP_W_PLMNSEARCH_MASK == (stDsdaPlmnSearchEnhancedCfg.usSolutionMask & NAS_MMC_UTRAN_SKIP_W_PLMNSEARCH_MASK))
    {
         NAS_MMC_SetUtranSkipWPlmnSearchFlag(VOS_TRUE);
    }

    if (NAS_MMC_SUPPORT_NCELL_SEARCH_MASK == (stDsdaPlmnSearchEnhancedCfg.usSolutionMask & NAS_MMC_SUPPORT_NCELL_SEARCH_MASK))
    {
        NAS_MMC_SetNcellSearchFlag(VOS_TRUE);
        NAS_MMC_SetNcellSearchFirstTimerLen(stDsdaPlmnSearchEnhancedCfg.ucSolution2NcellQuickSearchTimer);
        NAS_MMC_SetNcellSearchSecondTimerLen(stDsdaPlmnSearchEnhancedCfg.ucSolution2NcellQuickSearchTimer2);
    }

#endif

    return;
}




VOS_VOID  NAS_MMC_ReadIsrSupportNvim( VOS_VOID )
{
    VOS_UINT32                          ulLen;
    NAS_NVIM_ISR_CFG_STRU               stIsrCfg;

    PS_MEM_SET(&stIsrCfg, 0x00, sizeof(stIsrCfg));
    NV_GetLength(en_NV_Item_NasIsrSupport, &ulLen);

    /* 读NV项en_NV_Item_NasIsrSupport，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_NasIsrSupport,
                         &stIsrCfg, ulLen))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadIsrSupportNvim():WARNING: read en_NV_Item_NasIsrSupport Error");

        return;
    }

    if ( VOS_TRUE == stIsrCfg.ucIsrSupport)
    {
        NAS_MML_SetIsrSupportFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetIsrSupportFlg(VOS_FALSE);
    }

}

#if (FEATURE_ON == FEATURE_PTM)
VOS_VOID NAS_MMC_ReadErrlogCtrlInfoNvim(VOS_VOID)
{
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulLength;
    NV_ID_ERR_LOG_CTRL_INFO_STRU        stErrorLogCtrlInfo;

    ulLength = 0;
    PS_MEM_SET(&stErrorLogCtrlInfo, 0x00, sizeof(NV_ID_ERR_LOG_CTRL_INFO_STRU));
    NV_GetLength(en_NV_Item_ErrLogCtrlInfo, &ulLength);

    ulResult = NV_Read(en_NV_Item_ErrLogCtrlInfo,
                       &stErrorLogCtrlInfo,
                       ulLength);

    if (ulResult == NV_OK)
    {
        NAS_MML_SetErrlogCtrlFlag(stErrorLogCtrlInfo.ucAlmStatus);
        NAS_MML_SetErrlogAlmLevel(stErrorLogCtrlInfo.ucAlmLevel);
    }
    else
    {
        NAS_MML_SetErrlogCtrlFlag(VOS_FALSE);
        NAS_MML_SetErrlogAlmLevel(NAS_ERR_LOG_CTRL_LEVEL_CRITICAL);
    }

    return;
}
#endif


VOS_VOID NAS_MMC_ReadAccBarPlmnSearchNvim(VOS_VOID)
{
    VOS_UINT32                                              ulLen;
    NAS_MMC_NVIM_ACC_BAR_PLMN_SEARCH_FLG_STRU               stAccBarPlmnSearchFlg;

    PS_MEM_SET(&stAccBarPlmnSearchFlg, 0x00, sizeof(NAS_MMC_NVIM_ACC_BAR_PLMN_SEARCH_FLG_STRU));
    NV_GetLength(en_NV_Item_ACC_BAR_PLMN_SEARCH_FLG, &ulLen);

    /* 读NV项en_NV_Item_ACC_BAR_PLMN_SEARCH_FLG，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_ACC_BAR_PLMN_SEARCH_FLG,
                         &stAccBarPlmnSearchFlg, ulLen))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadAccBarPlmnSearchNvim():WARNING: read en_NV_Item_ACC_BAR_PLMN_SEARCH_FLG Error");

        return;
    }

    /* 赋值到全局变量 */
    if ( VOS_TRUE == stAccBarPlmnSearchFlg.ucAccBarPlmnSearchFlg)
    {
        NAS_MML_SetSupportAccBarPlmnSearchFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetSupportAccBarPlmnSearchFlg(VOS_FALSE);
    }

    return;
}



VOS_VOID NAS_MMC_ReadUserCfgOPlmnInfoNvim(VOS_VOID)
{
    VOS_UINT32                                              ulLen;
    VOS_UINT32                                              ulStep;
    VOS_UINT32                                              ulNvOplmnAvail;
    NAS_MML_PLMN_ID_STRU                                    stPlmn;
    NAS_MML_SIM_FORMAT_PLMN_ID                              stSimPlmn;
    NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU                *pstNvimCfgOPlmnInfo = VOS_NULL_PTR;
    NAS_MML_USER_CFG_OPLMN_INFO_STRU                       *pstUserCfgOPlmnInfo = VOS_NULL_PTR;

    PS_MEM_SET(&stPlmn, 0x00, sizeof(NAS_MML_PLMN_ID_STRU));
    PS_MEM_SET(&stSimPlmn, 0x00, sizeof(NAS_MML_SIM_FORMAT_PLMN_ID));

    pstNvimCfgOPlmnInfo = (NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU*)PS_MEM_ALLOC(
                                                      WUEPS_PID_MMC,
                                                      sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU));
    if (VOS_NULL_PTR == pstNvimCfgOPlmnInfo)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadUserCfgOPlmnInfoNvim():ERROR: Memory Alloc Error");
        return;
    }

    PS_MEM_SET(pstNvimCfgOPlmnInfo, 0x00, sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU));

    NV_GetLength(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST, &ulLen);

    pstUserCfgOPlmnInfo = NAS_MML_GetUserCfgOPlmnInfo();

    /* 读NV项en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST,
                         pstNvimCfgOPlmnInfo, ulLen))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadUserCfgOPlmnInfoNvim():WARNING: read en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST Error");
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
        return;
    }

    /* 确保ucActiveFlg值有效，为VOS_TRUE或VOS_FALSE */
    if (VOS_TRUE != pstNvimCfgOPlmnInfo->ucActiveFlg)
    {
         pstNvimCfgOPlmnInfo->ucActiveFlg = VOS_FALSE;
    }

    /* 确保ucImsiCheckFlg值有效，为VOS_TRUE或VOS_FALSE */
    if (VOS_FALSE != pstNvimCfgOPlmnInfo->ucImsiCheckFlg)
    {
         pstNvimCfgOPlmnInfo->ucImsiCheckFlg = VOS_TRUE;
    }

    /* 将NV中的PLMN LIST信息保存到MML的全局变量中去 */
    pstUserCfgOPlmnInfo->ucActiveFlg       = pstNvimCfgOPlmnInfo->ucActiveFlg;
    pstUserCfgOPlmnInfo->ucImsiCheckFlg    = pstNvimCfgOPlmnInfo->ucImsiCheckFlg;
    PS_MEM_CPY(pstUserCfgOPlmnInfo->aucVersion, pstNvimCfgOPlmnInfo->aucVersion, sizeof(pstUserCfgOPlmnInfo->aucVersion));

    /* 将NV中的OPLMN保存到内存中,需要防止数组越界 */
    if ( pstNvimCfgOPlmnInfo->usOplmnListNum > NAS_MML_MAX_USER_CFG_OPLMN_NUM )
    {
        pstNvimCfgOPlmnInfo->usOplmnListNum = NAS_MML_MAX_USER_CFG_OPLMN_NUM;
    }
    pstUserCfgOPlmnInfo->usOplmnListNum = pstNvimCfgOPlmnInfo->usOplmnListNum;

    /* 将NV中保存的IMSI信息转换为内存中的信息,需要防止数组越界 */
    if ( pstNvimCfgOPlmnInfo->ucImsiPlmnListNum > NAS_MML_MAX_USER_OPLMN_IMSI_NUM )
    {
        pstNvimCfgOPlmnInfo->ucImsiPlmnListNum = NAS_MML_MAX_USER_OPLMN_IMSI_NUM;
    }

    pstUserCfgOPlmnInfo->ucImsiPlmnListNum = 0;
    for ( ulStep = 0; ulStep < pstNvimCfgOPlmnInfo->ucImsiPlmnListNum; ulStep++ )
    {
        PS_MEM_CPY(stSimPlmn.aucSimPlmn, pstNvimCfgOPlmnInfo->astImsiPlmnList[ulStep].aucSimPlmn, NAS_MML_SIM_PLMN_ID_LEN);
        NAS_MMC_ConvertSimPlmnToNasPLMN(&stSimPlmn, &stPlmn);

        if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stPlmn))
        {
            pstUserCfgOPlmnInfo->astImsiPlmnList[pstUserCfgOPlmnInfo->ucImsiPlmnListNum] = stPlmn;
            pstUserCfgOPlmnInfo->ucImsiPlmnListNum++;
        }
    }

    /* 如果用户设置的OPLMN有效，需要将NV中的OPLMN列表保存到MML全局变量中 */
    ulNvOplmnAvail = NAS_MML_IsNvimOplmnAvail();

    if (VOS_TRUE == ulNvOplmnAvail)
    {
        /* 使用NV中的OPLMN文件覆盖MML中保存卡文件OPLMN信息的全局变量 */
        NAS_MMC_UpdateOPlmnInfoFromNvim(pstNvimCfgOPlmnInfo->aucOPlmnList, pstNvimCfgOPlmnInfo->usOplmnListNum);
    }

    PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
    return;
}


 VOS_VOID  NAS_MMC_ReadDplmnNplmnInfoNvim(VOS_VOID)
{
    VOS_UINT32                                              ulLen;
    NAS_MML_PLMN_ID_STRU                                    stHplmnId;
    VOS_UINT8                                               ucHplmnType;
    VOS_UINT8                                              *pucImsi         = VOS_NULL_PTR;
    NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU                 *pstNvimCfgDPlmnNPlmnInfo = VOS_NULL_PTR;
    NAS_MMC_DPLMN_NPLMN_CFG_INFO_STRU                      *pstDPlmnNPlmnCfgInfo = VOS_NULL_PTR;
    NAS_MMC_NVIM_CFG_DPLMN_NPLMN_FLAG_STRU                 *pstNvimCfgDPlmnNPlmnFlag = VOS_NULL_PTR;

    /* 取得手机卡中IMSI的信息 */
    pucImsi     = NAS_MML_GetSimImsi();

    /* 从当前的IMSI中取出home plmn */
    stHplmnId  = NAS_MML_GetImsiHomePlmn(pucImsi);

    pstNvimCfgDPlmnNPlmnFlag = (NAS_MMC_NVIM_CFG_DPLMN_NPLMN_FLAG_STRU*)PS_MEM_ALLOC(
                                                      WUEPS_PID_MMC,
                                                      sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_FLAG_STRU));
    if (VOS_NULL_PTR == pstNvimCfgDPlmnNPlmnFlag)
    {
        return;
    }

    pstNvimCfgDPlmnNPlmnInfo = (NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU*)PS_MEM_ALLOC(
                                                      WUEPS_PID_MMC,
                                                      sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));
    if (VOS_NULL_PTR == pstNvimCfgDPlmnNPlmnInfo)
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
        return;
    }

    pstDPlmnNPlmnCfgInfo = NAS_MMC_GetDPlmnNPlmnCfgInfo();

    PS_MEM_SET(pstNvimCfgDPlmnNPlmnFlag, 0x00, sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_FLAG_STRU));

    NV_GetLength(en_NV_Item_Cfg_Dplmn_Nplmn_Flag, &ulLen);

    if (ulLen > sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_FLAG_STRU))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_Cfg_Dplmn_Nplmn_Flag, pstNvimCfgDPlmnNPlmnFlag, ulLen))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
        return;
    }

    if (VOS_TRUE == pstNvimCfgDPlmnNPlmnFlag->usCfgDplmnNplmnFlag)
    {
        pstDPlmnNPlmnCfgInfo->ucActiveFlg = VOS_TRUE;
    }
    else
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
        return;
    }

    pstDPlmnNPlmnCfgInfo->ucCMCCHplmnNum = 0;
    pstDPlmnNPlmnCfgInfo->ucUNICOMHplmnNum = 0;
    pstDPlmnNPlmnCfgInfo->ucCTHplmnNum = 0;

    NAS_MMC_UpdateDPlmnNPlmnFlagInfo(pstNvimCfgDPlmnNPlmnFlag);

    PS_MEM_SET(pstNvimCfgDPlmnNPlmnInfo, 0x00, sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));

    ucHplmnType = NAS_MMC_JudegeHplmnType(&stHplmnId);

    /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-28, begin */
    if (NAS_MMC_HPLMN_TYPE_CMCC == ucHplmnType)
    {
        NV_GetLength(en_NV_Item_CMCC_Cfg_Dplmn_Nplmn_Info, &ulLen);

        if (ulLen > sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }

        /* 读NV项en_NV_Item_CMCC_Cfg_Dplmn_Nplmn_Info，失败，直接返回 */
        if (NV_OK != NV_Read(en_NV_Item_CMCC_Cfg_Dplmn_Nplmn_Info,
                             pstNvimCfgDPlmnNPlmnInfo, ulLen))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }
    }
    else if (NAS_MMC_HPLMN_TYPE_UNICOM == ucHplmnType)
    {
        NV_GetLength(en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn_Info, &ulLen);

        if (ulLen > sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }

        /* 读NV项en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn_Info，失败，直接返回 */
        if (NV_OK != NV_Read(en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn_Info,
                             pstNvimCfgDPlmnNPlmnInfo, ulLen))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }
    }
    else if (NAS_MMC_HPLMN_TYPE_CT == ucHplmnType)
    {
        NV_GetLength(en_NV_Item_CT_Cfg_Dplmn_Nplmn_Info, &ulLen);

        if (ulLen > sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }

        /* 读NV项en_NV_Item_CT_Cfg_Dplmn_Nplmn_Info，失败，直接返回 */
        if (NV_OK != NV_Read(en_NV_Item_CT_Cfg_Dplmn_Nplmn_Info,
                             pstNvimCfgDPlmnNPlmnInfo, ulLen))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }
    }
    else
    {
        pstDPlmnNPlmnCfgInfo->ucActiveFlg = VOS_FALSE;
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
        return;
    }
    /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-28, end */
    
    pstDPlmnNPlmnCfgInfo->usDplmnListNum = 0;
    pstDPlmnNPlmnCfgInfo->usNplmnListNum = 0;

    NAS_MMC_UpdateDplmnNplmnInfo(pstNvimCfgDPlmnNPlmnInfo);

    PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnFlag);
    PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
    return;
}
VOS_VOID NAS_MMC_WriteDplmnNplmnToNvim(VOS_VOID)
{
    VOS_UINT32                                              ulLen;
    VOS_UINT32                                              ulStep;
    NAS_MML_PLMN_ID_STRU                                    stUserPlmn;
    NAS_MML_SIM_FORMAT_PLMN_ID                              stNvimPlmn;
    NAS_MML_PLMN_ID_STRU                                    stHplmnId;
    VOS_UINT8                                               ucHplmnType;
    VOS_UINT8                                              *pucImsi = VOS_NULL_PTR;
    NAS_MMC_DPLMN_NPLMN_CFG_INFO_STRU                      *pstDPlmnNPlmnCfgInfo = VOS_NULL_PTR;
    NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU                 *pstNvimCfgDPlmnNPlmnInfo = VOS_NULL_PTR;

    PS_MEM_SET(&stHplmnId, 0, sizeof(stHplmnId));
    PS_MEM_SET(&stNvimPlmn, 0, sizeof(stNvimPlmn));
    PS_MEM_SET(&stUserPlmn, 0, sizeof(stUserPlmn));
    ulLen       = 0;
    ulStep      = 0;
    ucHplmnType = 0;

    /* 取得手机卡中IMSI的信息 */
    pucImsi    = NAS_MML_GetSimImsi();

    /* 从当前的IMSI中取出home plmn */
    stHplmnId  = NAS_MML_GetImsiHomePlmn(pucImsi);

    pstDPlmnNPlmnCfgInfo = NAS_MMC_GetDPlmnNPlmnCfgInfo();

    if (VOS_TRUE != pstDPlmnNPlmnCfgInfo->ucActiveFlg)
    {
        return;
    }

    pstNvimCfgDPlmnNPlmnInfo = (NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU*)PS_MEM_ALLOC(
                                   WUEPS_PID_MMC,
                                   sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));

    if (VOS_NULL_PTR == pstNvimCfgDPlmnNPlmnInfo)
    {
        return;
    }

    PS_MEM_SET(pstNvimCfgDPlmnNPlmnInfo, 0x00, sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));

    if ( pstDPlmnNPlmnCfgInfo->usDplmnListNum > NAS_MMC_MAX_CFG_DPLMN_NUM )
    {
        pstDPlmnNPlmnCfgInfo->usDplmnListNum = NAS_MMC_MAX_CFG_DPLMN_NUM;
    }

    pstNvimCfgDPlmnNPlmnInfo->usDplmnListNum = pstDPlmnNPlmnCfgInfo->usDplmnListNum;

    for ( ulStep = 0; ulStep < pstDPlmnNPlmnCfgInfo->usDplmnListNum; ulStep++ )
    {
        stNvimPlmn.aucSimPlmn[0] = 0;
        stNvimPlmn.aucSimPlmn[1] = 0;
        stNvimPlmn.aucSimPlmn[2] = 0;
        stUserPlmn.ulMcc = pstDPlmnNPlmnCfgInfo->astDPlmnList[ulStep].stSimPlmnWithRat.stPlmnId.ulMcc;
        stUserPlmn.ulMnc = pstDPlmnNPlmnCfgInfo->astDPlmnList[ulStep].stSimPlmnWithRat.stPlmnId.ulMnc;

        NAS_MMC_ConvertNasPlmnToSimFormat(&stUserPlmn, &stNvimPlmn);

        /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-20, begin */

        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN)]     = stNvimPlmn.aucSimPlmn[0];
        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 1] = stNvimPlmn.aucSimPlmn[1];
        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 2] = stNvimPlmn.aucSimPlmn[2];
        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 3] = (VOS_UINT8)((pstDPlmnNPlmnCfgInfo->astDPlmnList[ulStep].stSimPlmnWithRat.usSimRat) >> NAS_MML_OCTET_MOVE_EIGHT_BITS);
        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 4] = (VOS_UINT8)((pstDPlmnNPlmnCfgInfo->astDPlmnList[ulStep].stSimPlmnWithRat.usSimRat) & (0x00FF));

        /* 更新注册域信息 */
        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 5] = pstDPlmnNPlmnCfgInfo->astDPlmnList[ulStep].enRegDomain;

        /* 更新类型信息 */
        pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + NAS_MMC_DPLMN_NPLMN_NV_PRESETING_FLAG_INDEX] = pstDPlmnNPlmnCfgInfo->astDPlmnList[ulStep].enType;
        
        /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-20, end */
        
    }

    if ( pstDPlmnNPlmnCfgInfo->usNplmnListNum > NAS_MMC_MAX_CFG_NPLMN_NUM )
    {
        pstDPlmnNPlmnCfgInfo->usNplmnListNum = NAS_MMC_MAX_CFG_NPLMN_NUM;
    }

    pstNvimCfgDPlmnNPlmnInfo->usNplmnListNum = pstDPlmnNPlmnCfgInfo->usNplmnListNum;

    for ( ulStep = 0; ulStep < pstDPlmnNPlmnCfgInfo->usNplmnListNum; ulStep++ )
    {
        stNvimPlmn.aucSimPlmn[0]= 0;
        stNvimPlmn.aucSimPlmn[1]= 0;
        stNvimPlmn.aucSimPlmn[2]= 0;
        stUserPlmn.ulMcc        = pstDPlmnNPlmnCfgInfo->astNPlmnList[ulStep].stSimPlmnWithRat.stPlmnId.ulMcc;
        stUserPlmn.ulMnc        = pstDPlmnNPlmnCfgInfo->astNPlmnList[ulStep].stSimPlmnWithRat.stPlmnId.ulMnc;

        NAS_MMC_ConvertNasPlmnToSimFormat(&stUserPlmn, &stNvimPlmn);

        /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-20, begin */
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN)]     = stNvimPlmn.aucSimPlmn[0];
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 1] = stNvimPlmn.aucSimPlmn[1];
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 2] = stNvimPlmn.aucSimPlmn[2];
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 3] = (VOS_UINT8)((pstDPlmnNPlmnCfgInfo->astNPlmnList[ulStep].stSimPlmnWithRat.usSimRat) >> NAS_MML_OCTET_MOVE_EIGHT_BITS);
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 4] = (VOS_UINT8)((pstDPlmnNPlmnCfgInfo->astNPlmnList[ulStep].stSimPlmnWithRat.usSimRat) & (0x00FF));

        /* 更新注册域信息 */
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + 5] = pstDPlmnNPlmnCfgInfo->astNPlmnList[ulStep].enRegDomain;

        /* 更新类型信息 */
        pstNvimCfgDPlmnNPlmnInfo->aucNPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + NAS_MMC_DPLMN_NPLMN_NV_PRESETING_FLAG_INDEX] = pstDPlmnNPlmnCfgInfo->astNPlmnList[ulStep].enType;
    
    /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-20, end */
        
    }

    ucHplmnType = NAS_MMC_JudegeHplmnType(&stHplmnId);

    /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-28, begin */
    if (NAS_MMC_HPLMN_TYPE_CMCC == ucHplmnType)
    {
        NV_GetLength(en_NV_Item_CMCC_Cfg_Dplmn_Nplmn_Info, &ulLen);

        /* 写NV项en_NV_Item_CMCC_Cfg_Dplmn_Nplmn，失败，直接返回 */
        if (NV_OK != NV_Write(en_NV_Item_CMCC_Cfg_Dplmn_Nplmn_Info,
                              pstNvimCfgDPlmnNPlmnInfo, ulLen))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }
    }

    if (NAS_MMC_HPLMN_TYPE_UNICOM == ucHplmnType)
    {
        NV_GetLength(en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn_Info, &ulLen);
        /* 写NV项en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn，失败，直接返回 */
        if (NV_OK != NV_Write(en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn_Info,
                              pstNvimCfgDPlmnNPlmnInfo, ulLen))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }
    }

    if (NAS_MMC_HPLMN_TYPE_CT == ucHplmnType)
    {
        NV_GetLength(en_NV_Item_CT_Cfg_Dplmn_Nplmn_Info, &ulLen);
        /* 写NV项en_NV_Item_CT_Cfg_Dplmn_Nplmn，失败，直接返回 */
        if (NV_OK != NV_Write(en_NV_Item_CT_Cfg_Dplmn_Nplmn_Info,
                              pstNvimCfgDPlmnNPlmnInfo, ulLen))
        {
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
            return;
        }
    }
    /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-28, end */
    
    PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
    return;

}
NAS_MMC_HPLMN_TYPE_ENUM_UINT8 NAS_MMC_JudegeHplmnType(
    NAS_MML_PLMN_ID_STRU               *pstHplmnId
)
{
    NAS_MMC_DPLMN_NPLMN_CFG_INFO_STRU                      *pstDPlmnNPlmnCfgInfo = VOS_NULL_PTR;
    VOS_UINT32                          i;

    pstDPlmnNPlmnCfgInfo = NAS_MMC_GetDPlmnNPlmnCfgInfo();

    for (i = 0; i < pstDPlmnNPlmnCfgInfo->ucCMCCHplmnNum; i++)
    {
        if (VOS_TRUE == NAS_MML_CompareBcchPlmnwithSimPlmn(&pstDPlmnNPlmnCfgInfo->astCMCCHplmnList[i], pstHplmnId))
        {
            return NAS_MMC_HPLMN_TYPE_CMCC;
        }
    }

    for (i = 0; i < pstDPlmnNPlmnCfgInfo->ucUNICOMHplmnNum; i++)
    {
        if (VOS_TRUE == NAS_MML_CompareBcchPlmnwithSimPlmn(&pstDPlmnNPlmnCfgInfo->astUNICOMHplmnList[i], pstHplmnId))
        {
            return NAS_MMC_HPLMN_TYPE_UNICOM;
        }
    }

    for (i = 0; i < pstDPlmnNPlmnCfgInfo->ucCTHplmnNum; i++)
    {
        if (VOS_TRUE == NAS_MML_CompareBcchPlmnwithSimPlmn(&pstDPlmnNPlmnCfgInfo->astCTHplmnList[i], pstHplmnId))
        {
            return NAS_MMC_HPLMN_TYPE_CT;
        }
    }

    return NAS_MMC_HPLMN_TYPE_BUTT;
}


#if (FEATURE_ON == FEATURE_DSDS)

VOS_VOID NAS_MMC_ReadModemRfShareCfgNvim(VOS_VOID)
{
    NV_MODEM_RF_SHARE_CFG_STRU          stNvimRfShareCfgInfo;

    PS_MEM_SET(&stNvimRfShareCfgInfo, 0x00, sizeof(NV_MODEM_RF_SHARE_CFG_STRU));

    /* 读NV项en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_MODEM_RF_SHARE_CFG,
                         &stNvimRfShareCfgInfo, sizeof(NV_MODEM_RF_SHARE_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadModemRfShareCfgNvim():WARNING: read en_NV_MODEM_RF_SHARE_CFG Error");
        
        return;
    }

    if ( (VOS_TRUE  != stNvimRfShareCfgInfo.usSupportFlag)
      && (VOS_FALSE != stNvimRfShareCfgInfo.usSupportFlag) )
    {
        /* NV内容异常保护 */
        stNvimRfShareCfgInfo.usSupportFlag = VOS_FALSE;
    }

    NAS_MML_SetDsdsRfShareFlg(stNvimRfShareCfgInfo.usSupportFlag);
    
    return;
}

#endif

VOS_VOID  NAS_MMC_ReadUmtsCodecTypeNvim(VOS_VOID)
{
    MN_CALL_UMTS_CODEC_TYPE_STRU        stNvimCodecType;
    NAS_MML_CALL_UMTS_CODEC_TYPE_STRU   stCodecType; 

    PS_MEM_SET(&stNvimCodecType, 0x00, sizeof(stNvimCodecType));
    PS_MEM_SET(&stCodecType, 0x00, sizeof(stCodecType));

    if ( NV_OK != NV_Read(en_NV_Item_UMTS_CODEC_TYPE,
                          &stNvimCodecType,
                          sizeof(stNvimCodecType)))
    {
        return;
    }

    stCodecType.ucCnt = stNvimCodecType.ucCnt;
    PS_MEM_CPY(stCodecType.aucUmtsCodec, stNvimCodecType.aucUmtsCodec, sizeof(stCodecType.aucUmtsCodec));

    NAS_MML_CALL_SetCallUmtsCodecType(&stCodecType);
}


VOS_VOID  NAS_MMC_ReadMedCodecTypeNvim(VOS_VOID)
{
    MN_CALL_NIMV_ITEM_CODEC_TYPE_STRU   stNvimCodecType;
    NAS_MML_CALL_GSM_CODEC_TYPE_STRU    stCodecType;

    PS_MEM_SET(&stNvimCodecType, 0x00, sizeof(stNvimCodecType));  
    PS_MEM_SET(&stCodecType, 0x00, sizeof(stCodecType)); 

    if ( NV_OK != NV_Read(en_NV_Item_MED_CODEC_TYPE,
                          &stNvimCodecType,
                          sizeof(stNvimCodecType)))
    {
        return;
    }

    stCodecType.ucCodecTypeNum = stNvimCodecType.ucCodecTypeNum;
    PS_MEM_CPY(stCodecType.aucCodecType, stNvimCodecType.aucCodecType, sizeof(stNvimCodecType.aucCodecType));
    
    NAS_MML_CALL_SetCallGsmCodecType(&stCodecType);
}

VOS_VOID NAS_MMC_ReadImsVoiceMMEnableNvim(VOS_VOID)
{
    NAS_MMC_IMS_VOICE_MOBILE_MANAGEMENT stImsVoiceMM;
    VOS_UINT32                          ulLength;

    ulLength = 0;

    NV_GetLength(en_NV_Item_IMS_VOICE_MOBILE_MANAGEMENT, &ulLength);
    if (ulLength > sizeof(stImsVoiceMM))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImsVoiceMMEnableNvim():WARNING: NvGetLength too long");
        return;
    }

    if (NV_OK != NV_Read(en_NV_Item_IMS_VOICE_MOBILE_MANAGEMENT,
                         &stImsVoiceMM, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImsVoiceMMEnableNvim():WARNING: read en_NV_Item_IMS_VOICE_MOBILE_MANAGEMENT Error");
        return;
    }

    if (VOS_TRUE  == stImsVoiceMM.ucImsVoiceInterSysLauEnable)
    {
        NAS_MML_SetImsVoiceInterSysLauEnableFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetImsVoiceInterSysLauEnableFlg(VOS_FALSE);
    }

    if (VOS_TRUE  == stImsVoiceMM.ucImsVoiceMMEnable)
    {
        NAS_MML_SetImsVoiceMMEnableFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetImsVoiceMMEnableFlg(VOS_FALSE);
    }

    return;
}


VOS_VOID NAS_MMC_ReadLcCtrlParaNvim(VOS_VOID)
{
    TAF_NV_LC_CTRL_PARA_STRU            stLcCtrlPara;

    PS_MEM_SET(&stLcCtrlPara, 0, sizeof(stLcCtrlPara));

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_LC_Ctrl_PARA,
                         &stLcCtrlPara,
                         sizeof(stLcCtrlPara)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLcCtrlParaNvim(): en_NV_Item_LC_Ctrl_PARA error");
        return;
    }

    if (VOS_TRUE == stLcCtrlPara.ucLCEnableFlg)
    {
        NAS_MML_SetLcEnableFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetLcEnableFlg(VOS_FALSE);
    }

    return;
}
VOS_VOID NAS_MMC_ReadIgnoreAuthRejFlgNvim(VOS_VOID)
{
    VOS_UINT32                                              ulLen;
    NAS_MMC_NVIM_IGNORE_AUTH_REJ_CFG_STRU               stIgnoreAuthRejInfo;

    PS_MEM_SET(&stIgnoreAuthRejInfo, 0x00, sizeof(NAS_MMC_NVIM_IGNORE_AUTH_REJ_CFG_STRU));
    NV_GetLength(en_NV_Item_Ignore_Auth_Rej_CFG, &ulLen);

    stIgnoreAuthRejInfo.ucIgnoreAuthRejFlg = VOS_FALSE;
    stIgnoreAuthRejInfo.ucMaxAuthRejNo = 0;
    /* 读NV项en_NV_Item_Remove_Auth_Rej_CFG，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_Ignore_Auth_Rej_CFG,
                         &stIgnoreAuthRejInfo, ulLen))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadIgnoreAuthRejFlgNvim():WARNING: read en_NV_Item_Remove_Auth_Rej_CFG Error");

        return;
    }

    if (VOS_TRUE == NAS_USIMMAPI_IsTestCard())
    {
        NAS_MML_SetIgnoreAuthRejFlg(VOS_FALSE);
        return;
    }

    /* 赋值到全局变量 */
    if ( VOS_TRUE == stIgnoreAuthRejInfo.ucIgnoreAuthRejFlg)
    {
        NAS_MML_SetIgnoreAuthRejFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetIgnoreAuthRejFlg(VOS_FALSE);
    }

    NAS_MML_SetMaxAuthRejNo((stIgnoreAuthRejInfo.ucMaxAuthRejNo));

    return;
}



VOS_VOID NAS_MMC_ReadHighPrioRatHplmnTimerCfgNvim( VOS_VOID  )
{
    NAS_MML_HIGH_PRIO_RAT_HPLMN_TIMER_CFG_STRU       *pstHighRatHplmnTimerCfg = VOS_NULL_PTR;
    NAS_MMC_NVIM_HIGH_PRIO_RAT_HPLMN_TIMER_CFG_STRU   stHighRatHplmnTimerCfg;

    PS_MEM_SET(&stHighRatHplmnTimerCfg, 0x00, sizeof(stHighRatHplmnTimerCfg));

    pstHighRatHplmnTimerCfg = NAS_MML_GetHighPrioRatHplmnTimerCfg();

    if (NV_OK != NV_Read (en_NV_Item_HIGH_PRIO_RAT_HPLMN_TIMER_CFG, &stHighRatHplmnTimerCfg, sizeof(stHighRatHplmnTimerCfg)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadHighPrioRatHplmnTimerCfgNvim:Read NV Failed");
        return;
    }

    pstHighRatHplmnTimerCfg->ucActiveFLg = VOS_FALSE;
    
    if (VOS_TRUE == stHighRatHplmnTimerCfg.ucActiveFLg)
    {
        pstHighRatHplmnTimerCfg->ucActiveFLg                = VOS_TRUE;
        pstHighRatHplmnTimerCfg->ucTdThreshold              = stHighRatHplmnTimerCfg.ucTdThreshold;
        pstHighRatHplmnTimerCfg->ulFirstSearchTimeLen       = stHighRatHplmnTimerCfg.ulFirstSearchTimeLen;
        pstHighRatHplmnTimerCfg->ulNonFirstSearchTimeLen    = stHighRatHplmnTimerCfg.ulNonFirstSearchTimeLen;
        pstHighRatHplmnTimerCfg->ulFirstSearchTimeCount     = stHighRatHplmnTimerCfg.ulFirstSearchTimeCount;
        pstHighRatHplmnTimerCfg->ulRetrySearchTimeLen       = stHighRatHplmnTimerCfg.ulRetrySearchTimeLen;
    }
    
    return;
}

#if  (FEATURE_ON == FEATURE_LTE)

VOS_VOID NAS_MMC_ReadUltraFlashCsfbSupportFlgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_ULTRA_FLASH_CSFB_SUPPORT_FLG_STRU          stUltraFlashCsfbSupport;

    PS_MEM_SET(&stUltraFlashCsfbSupport, 0x00, sizeof(NAS_MMC_NVIM_ULTRA_FLASH_CSFB_SUPPORT_FLG_STRU));

    /* 读NV项en_NV_Item_Ultra_Flash_Csfb_Support_Flg，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_Ultra_Flash_Csfb_Support_Flg,
                         &stUltraFlashCsfbSupport, 
                         sizeof(NAS_MMC_NVIM_ULTRA_FLASH_CSFB_SUPPORT_FLG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadIgnoreAuthRejFlgNvim():WARNING: read en_NV_Item_Remove_Auth_Rej_CFG Error");

        return;
    }

    /* 赋值到全局变量 */
    if ( VOS_TRUE == stUltraFlashCsfbSupport.ucUltraFlashCsfbSupportFLg)
    {
        NAS_MML_SetUltraFlashCsfbSupportFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetUltraFlashCsfbSupportFlg(VOS_FALSE);
    }

    return;
}


VOS_VOID NAS_MMC_ReadSrvccSupportFlgNvim(VOS_VOID)
{
    IMS_NV_IMS_CAP_STRU                 stImsCap;
    VOS_UINT8                           ucSrvccFlg;
    NAS_MML_MS_CAPACILITY_INFO_STRU    *pstMsCapability = VOS_NULL_PTR;

    pstMsCapability = NAS_MML_GetMsCapability();

    PS_MEM_SET(&stImsCap, 0x00, sizeof(IMS_NV_IMS_CAP_STRU));

    /* 读NV项EN_NV_ID_IMS_CAPABILITY，失败，直接返回 */
    if (NV_OK != NV_Read(EN_NV_ID_IMS_CAPABILITY,
                         &stImsCap, 
                         sizeof(IMS_NV_IMS_CAP_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSrvccSupportFlgNvim():WARNING: read EN_NV_ID_IMS_CAPABILITY Error");

        return;
    }

    /* active/hold/alerting/pre alterting任一个状态支持SRVCC，就给网络报支持SRVCC */
    ucSrvccFlg  = (stImsCap.ucSrvccOnImsSupportFlag || stImsCap.ucSrvccMidCallOnImsSupportFlag 
                || stImsCap.ucSrvccAlertingOnImsSupportFlag || stImsCap.ucSrvccPreAlertingOnImsSupportFlag);

    /* 赋值到全局变量，aucNetworkCapability中srvcc的bit位所在的位置参考3GPP 24008 */
    if ( VOS_TRUE == ucSrvccFlg)
    {
        NAS_MML_SetSupportSrvccFlg(VOS_TRUE);
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[2] |= 0x08;
    }
    else
    {
        NAS_MML_SetSupportSrvccFlg(VOS_FALSE);
        pstMsCapability->stMsNetworkCapability.aucNetworkCapability[2] &= 0xf7;
    }

    return;
}


#endif
VOS_VOID NAS_MMC_Read3GPP2UplmnNotPrefFlgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_3GPP2_UPLMN_NOT_PREF_STRU                  st3GPP2UplmnNotPref;

    PS_MEM_SET(&st3GPP2UplmnNotPref, 0x00, sizeof(NAS_MMC_NVIM_3GPP2_UPLMN_NOT_PREF_STRU));

    /* 读NV项en_NV_Item_3GPP2_Uplmn_Not_Pref_Flg，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_3GPP2_Uplmn_Not_Pref_Flg,
                         &st3GPP2UplmnNotPref, 
                         sizeof(NAS_MMC_NVIM_3GPP2_UPLMN_NOT_PREF_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_Read3GPP2UplmnNotPrefFlgNvim():WARNING: read en_NV_Item_3GPP2_Uplmn_Not_Pref_Flg Error");

        return;
    }

    /* 赋值到全局变量 */
    if (VOS_TRUE == st3GPP2UplmnNotPref.uc3GPP2UplmnNotPrefFlg)
    {
        NAS_MML_Set3GPP2UplmnNotPrefFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_Set3GPP2UplmnNotPrefFlg(VOS_FALSE);
    }

    return;
}


VOS_VOID NAS_MMC_ReadDamCfgNvim(VOS_VOID)
{
    PS_NV_DAM_CONFIG_PARA_STRU                  stDamCfgInfo;
    VOS_UINT32                                  ulLength;
    VOS_UINT8                                   i;
    NAS_MML_DAM_PLMN_INFO_CONFIG_STRU          *pstDamPlmnInfoCfg = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU                        stGUNasPlmn;
    MMC_LMM_PLMN_ID_STRU                        stLMMPlmn;

    PS_MEM_SET(&stDamCfgInfo, 0x00, sizeof(stDamCfgInfo));
    PS_MEM_SET(&stGUNasPlmn, 0x00, sizeof(stGUNasPlmn));
    PS_MEM_SET(&stLMMPlmn, 0x00, sizeof(stLMMPlmn));
    ulLength              = 0;
    pstDamPlmnInfoCfg     = NAS_MML_GetDamPlmnInfoCfg();

    /* 先获取NV的长度 */
    NV_GetLength(EN_NV_ID_DAM_CONFIG_PARA, &ulLength);

    if (ulLength > sizeof(PS_NV_DAM_CONFIG_PARA_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDamCfgNvim(): nv length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(EN_NV_ID_DAM_CONFIG_PARA,
                         &stDamCfgInfo, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadDamCfgNvim(): read nv error");
        return;
    }

    /* 获取enable lte是否需要搜网设置 */
    if (VOS_TRUE == stDamCfgInfo.ucEnableLteTrigPlmnSearchFlag)
    {
        NAS_MML_SetEnableLteTrigPlmnSearchFlag(stDamCfgInfo.ucEnableLteTrigPlmnSearchFlag);
    }

    if (stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.ulPlmnNum > NAS_MML_SUPPORT_DAM_MAX_PLMN_NUM)
    {
        stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.ulPlmnNum = NAS_MML_SUPPORT_DAM_MAX_PLMN_NUM;
    }

    if (stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.ulPlmnNum  > NAS_MML_SUPPORT_DAM_MAX_PLMN_NUM)
    {
        stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.ulPlmnNum = NAS_MML_SUPPORT_DAM_MAX_PLMN_NUM;
    }

    pstDamPlmnInfoCfg->stDamPlmnList.ucPlmnNum = (VOS_UINT8)stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.ulPlmnNum;

    for (i = 0; i < stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.ulPlmnNum; i++)
    {
        stLMMPlmn.aucPlmnId[0] = stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.astPlmnId[i].aucPlmnId[0];
        stLMMPlmn.aucPlmnId[1] = stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.astPlmnId[i].aucPlmnId[1];
        stLMMPlmn.aucPlmnId[2] = stDamCfgInfo.stDamConfigPlmnPara.stDamPlmnList.astPlmnId[i].aucPlmnId[2];

        NAS_MMC_ConvertLmmPlmnToGUNasFormat(&stLMMPlmn, &stGUNasPlmn);

        if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stGUNasPlmn))
        {
            pstDamPlmnInfoCfg->stDamPlmnList.astPlmnId[i].ulMcc = stGUNasPlmn.ulMcc;
            pstDamPlmnInfoCfg->stDamPlmnList.astPlmnId[i].ulMnc = stGUNasPlmn.ulMnc;
        }
    }

    pstDamPlmnInfoCfg->stImsiPlmnList.ucPlmnNum = (VOS_UINT8)stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.ulPlmnNum;

    for (i = 0; i < stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.ulPlmnNum; i++)
    {
        stLMMPlmn.aucPlmnId[0] = stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.astPlmnId[i].aucPlmnId[0];
        stLMMPlmn.aucPlmnId[1] = stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.astPlmnId[i].aucPlmnId[1];
        stLMMPlmn.aucPlmnId[2] = stDamCfgInfo.stDamConfigPlmnPara.stImsiPlmnList.astPlmnId[i].aucPlmnId[2];

        NAS_MMC_ConvertLmmPlmnToGUNasFormat(&stLMMPlmn, &stGUNasPlmn);

        if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stGUNasPlmn))
        {
            pstDamPlmnInfoCfg->stImsiPlmnList.astPlmnId[i].ulMcc = stGUNasPlmn.ulMcc;
            pstDamPlmnInfoCfg->stImsiPlmnList.astPlmnId[i].ulMnc = stGUNasPlmn.ulMnc;
        }
    }

    /* 获取在DAM网络disable lte后是否需要将DAM网络加入禁止接入技术网络列表中 */
    if (VOS_TRUE == stDamCfgInfo.ucAddDamPlmnInDisablePlmnWithRatListFlag)
    {
        NAS_MML_SetAddDamPlmnInDisablePlmnWithRatListFlag(VOS_TRUE);
    }

    return;
}





VOS_VOID NAS_MMC_ReadSyscfgTriPlmnSrchCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_SYSCFG_TRIGGER_PLMN_SEARCH_CFG_STRU        stSyscfgTriPlmnSrchCfg;
    VOS_UINT32                                              ulLength;

    ulLength = 0;
    PS_MEM_SET(&stSyscfgTriPlmnSrchCfg, 0x00, sizeof(NAS_MMC_NVIM_SYSCFG_TRIGGER_PLMN_SEARCH_CFG_STRU));

    /* 先获取NV的长度 */
    NV_GetLength(en_NV_Item_Syscfg_Trigger_Plmn_Search_Cfg, &ulLength);

    if (ulLength > sizeof(NAS_MMC_NVIM_SYSCFG_TRIGGER_PLMN_SEARCH_CFG_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSyscfgTriPlmnSrchCfgNvim(): en_NV_Item_Syscfg_Trigger_Plmn_Search_Cfg length Error");
        return;
    }

    /* 读NV失败 */
    if (NV_OK != NV_Read(en_NV_Item_Syscfg_Trigger_Plmn_Search_Cfg,
                         &stSyscfgTriPlmnSrchCfg, ulLength))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadSyscfgTriPlmnSrchCfgNvim(): en_NV_Item_Syscfg_Trigger_Plmn_Search_Cfg error");
        return;
    }

    if (VOS_TRUE == stSyscfgTriPlmnSrchCfg.ucHighPrioRatPlmnSrchFlg)
    {
        NAS_MML_SetSyscfgTriHighRatSrchFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetSyscfgTriHighRatSrchFlg(VOS_FALSE);
    }

    return;
}


VOS_VOID  NAS_MMC_ReadChangeNwCauseCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_CHANGE_NW_CAUSE_CFG_STRU                   stNvConfigCause;
    VOS_UINT32                                              ulRslt;

    PS_MEM_SET(&stNvConfigCause, 0x00, sizeof(NAS_MMC_NVIM_CHANGE_NW_CAUSE_CFG_STRU));

    ulRslt = NV_Read(en_NV_Item_ChangeNWCause_CFG,  
                     (VOS_VOID *)&stNvConfigCause,  
                     sizeof(NAS_MMC_NVIM_CHANGE_NW_CAUSE_CFG_STRU));

    if (NV_OK == ulRslt)
    {   
        if (NAS_MMC_NVIM_MAX_CAUSE_NUM < stNvConfigCause.ucCsRegCauseNum)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV para ucCsRegCauseNum err.");
            return;
        }

        if (NAS_MMC_NVIM_MAX_CAUSE_NUM < stNvConfigCause.ucPsRegCauseNum)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV para ucPsRegCauseNum err.");
            return;
        }

        if (NAS_MMC_NVIM_MAX_CAUSE_NUM < stNvConfigCause.ucDetachCauseNum)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV para ucDetachCauseNum err.");
            return;
        }

        if (NAS_MMC_NVIM_MAX_CAUSE_NUM < stNvConfigCause.ucPsSerRejCauseNum)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV para ucPsSerRejCauseNum err.");
            return;
        }

        if (NAS_MMC_NVIM_MAX_CAUSE_NUM < stNvConfigCause.ucMmAbortCauseNum)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV para ucMmAbortCauseNum err.");
            return;
        }

        if (NAS_MMC_NVIM_MAX_CAUSE_NUM < stNvConfigCause.ucCmSerRejCauseNum)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV para ucCmSerRejCauseNum err.");
            return;
        }   
    }
    else
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadChangeNwCauseCfgNvim(): config NV err.");
        
        PS_MEM_SET(&stNvConfigCause, 0x00, sizeof(NAS_MMC_NVIM_CHANGE_NW_CAUSE_CFG_STRU));
    }

    /* 保存NV配置信息 */
    NAS_MML_SetChangeNWCauseCfg(&stNvConfigCause);
}
VOS_VOID NAS_MMC_ReadRelPsSignalConCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_REL_PS_SIGNAL_CON_CFG_STRU                 stRelPsSigConCfg;

    PS_MEM_SET(&stRelPsSigConCfg, 0x00, sizeof(NAS_MMC_NVIM_REL_PS_SIGNAL_CON_CFG_STRU));

    if(NV_OK != NV_Read(en_NV_Item_REL_PS_SIGNAL_CON_CFG,
                        (VOS_VOID *)&stRelPsSigConCfg, 
                        sizeof(NAS_MMC_NVIM_REL_PS_SIGNAL_CON_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadRelPsSignalConCfgNvim():en_NV_Item_REL_PS_SIGNAL_CON_CFG Error");
        return;
    }

    if(VOS_FALSE == stRelPsSigConCfg.ucRelPsSignalConFlg)
    {
        NAS_MML_SetRelPsSigConFlg(VOS_FALSE);
        return;
    }

    NAS_MML_SetRelPsSigConFlg(VOS_TRUE);
    NAS_MML_SetRelPsSigConCfg_T3340TimerLen(stRelPsSigConCfg.ulT3340Len);
}

#if  (FEATURE_ON == FEATURE_LTE)

VOS_VOID NAS_MMC_ReadLteRejCause14CfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_LTE_REJ_CAUSE_14_CFG_STRU    stLteRejCause14Cfg;

    PS_MEM_SET(&stLteRejCause14Cfg, 0x00, sizeof(NAS_MMC_NVIM_LTE_REJ_CAUSE_14_CFG_STRU));
    
    if (NV_OK != NV_Read(en_NV_Item_LTE_REJ_CAUSE_14_CFG,
                         (VOS_VOID *)&stLteRejCause14Cfg, 
                         sizeof(NAS_MMC_NVIM_LTE_REJ_CAUSE_14_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadLteRejCause14CfgNvim():en_NV_Item_LTE_REJ_CAUSE_14_CFG Error");
        return;
    }

    /* 一致性测试卡，或功能未开启 */
    if (   (VOS_TRUE  == NAS_USIMMAPI_IsTestCard())
        || (VOS_FALSE == stLteRejCause14Cfg.ucLteRejCause14Flg))
    {
        NAS_MML_SetLteRejCause14Flg(VOS_FALSE);
        return;
    }
    
    NAS_MML_SetLteRejCause14Flg(VOS_TRUE);
    NAS_MML_SetLteRejCause14EnableLteTimerLen(stLteRejCause14Cfg.usLteRejCause14EnableLteTimerLen * NAS_MML_SIXTY_SECOND
                                 * NAS_MML_ONE_THOUSAND_MILLISECOND);
}
#endif
VOS_VOID NAS_MMC_ReadCsmoSupportedCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_CSMO_SUPPORTED_CFG_STRU  stNvimCfgCsmoInfo;

    PS_MEM_SET(&stNvimCfgCsmoInfo, 0x00, sizeof(stNvimCfgCsmoInfo));

    if (NV_OK != NV_Read(en_NV_Item_Csmo_Supported_Cfg_Info,
                        (VOS_VOID *)&stNvimCfgCsmoInfo, 
                        sizeof(NAS_MMC_NVIM_CSMO_SUPPORTED_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadCsmoSupportedCfgNvim():en_NV_Item_Csmo_Supported_Cfg_Info Error");
        return;
    }

    if (VOS_TRUE == stNvimCfgCsmoInfo.ucCsmoSupportedFlg)
    {
        NAS_MML_SetCsmoSupportedFlg(VOS_TRUE);        
        
        return;
    }

    NAS_MML_SetCsmoSupportedFlg(VOS_FALSE);
    
    return;
}


VOS_VOID NAS_MMC_ReadT3212TimerCfgNvim(VOS_VOID)
{
    NAS_MML_T3212_TIMER_INFO_CONFIG_STRU                   *pstT3212Info        = VOS_NULL_PTR;    
    NAS_MMC_NVIM_T3212_TIMER_CFG_STRU                       stNvimCfgT3212Info;

    pstT3212Info = NAS_MML_GetT3212TimerInfo();
    PS_MEM_SET(&stNvimCfgT3212Info, 0x00, sizeof(stNvimCfgT3212Info));

    if (NV_OK != NV_Read(en_NV_Item_T3212_Timer_Cfg_Info,
                        (VOS_VOID *)&stNvimCfgT3212Info, 
                        sizeof(NAS_MMC_NVIM_T3212_TIMER_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadT3212TimerCfgNvim():en_NV_Item_Csmo_Supported_Cfg_Info Error");
        return;
    }

    pstT3212Info->ulT3212NvActiveFlg = VOS_FALSE;

    /* 对NV配置的异常值进行兼容处理, 配置T3212时长为0，则修改为默认6分钟 */
    if (0 == stNvimCfgT3212Info.ulT3212Phase1TimeLen)
    {
        stNvimCfgT3212Info.ulT3212Phase1TimeLen = NAS_MML_T3212_DEFAULT_UNIT_LEN;
    }

    /* 对NV配置的异常值进行兼容处理, 配置T3212时长为0，则修改为默认6分钟 */
    if (0 == stNvimCfgT3212Info.ulT3212Phase2TimeLen)
    {
        stNvimCfgT3212Info.ulT3212Phase2TimeLen = NAS_MML_T3212_DEFAULT_UNIT_LEN;
    }

    /* NV配置的T3212最大定时器不超过18小时 */
    if (stNvimCfgT3212Info.ulT3212Phase1TimeLen >= NAS_MML_T3212_CFG_MAX_VALUE)
    {
        stNvimCfgT3212Info.ulT3212Phase1TimeLen = NAS_MML_T3212_CFG_MAX_VALUE;
    }

    /* NV配置的T3212最大定时器不超过18小时 */
    if (stNvimCfgT3212Info.ulT3212Phase2TimeLen >= NAS_MML_T3212_CFG_MAX_VALUE)
    {
        stNvimCfgT3212Info.ulT3212Phase2TimeLen = NAS_MML_T3212_CFG_MAX_VALUE;
    }
    
    if (VOS_TRUE == stNvimCfgT3212Info.ulNvActiveFlg)
    {
        pstT3212Info->ulT3212NvActiveFlg = VOS_TRUE;
    }

    pstT3212Info->ulT3212StartSceneCtrlBitMask  = stNvimCfgT3212Info.ulT3212StartSceneCtrlBitMask;
    pstT3212Info->ulT3212Phase1TimeLen          = stNvimCfgT3212Info.ulT3212Phase1TimeLen * NAS_MML_ONE_THOUSAND_MILLISECOND;
    pstT3212Info->ulT3212Phase1Count            = stNvimCfgT3212Info.ulT3212Phase1Count;
    pstT3212Info->ulT3212Phase2TimeLen          = stNvimCfgT3212Info.ulT3212Phase2TimeLen * NAS_MML_ONE_THOUSAND_MILLISECOND;
    pstT3212Info->ulT3212Phase2Count            = stNvimCfgT3212Info.ulT3212Phase2Count;
    
    return;
}



VOS_VOID NAS_MMC_ReadRoamDisplayCfgNvim(VOS_VOID)
{
    NAS_MMC_NVIM_ROAM_DISPLAY_CFG_STRU          stNvimRoamDisplayCfg;;

    PS_MEM_SET(&stNvimRoamDisplayCfg, 0x00, sizeof(stNvimRoamDisplayCfg));

    if (NV_OK != NV_Read(en_NV_Item_Roam_Display_Cfg,
                        (VOS_VOID *)&stNvimRoamDisplayCfg,
                        sizeof(NAS_MMC_NVIM_ROAM_DISPLAY_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadRoamDisplayCfgNvim():en_NV_Item_Roam_Display_Cfg Error");
        return;
    }

    if (VOS_TRUE == stNvimRoamDisplayCfg.ucHplmnInEplmnDisplayHomeFlg)
    {
        NAS_MML_SetHplmnInEplmnDisplayHomeFlg(VOS_TRUE);

        return;
    }

    NAS_MML_SetHplmnInEplmnDisplayHomeFlg(VOS_FALSE);

    return;
}


#if  (FEATURE_ON == FEATURE_IMS)

VOS_VOID NAS_MMC_ReadImsRatSupportNvim(VOS_VOID)
{
    VOS_UINT32                                  ulLength;
    IMSA_NV_IMS_RAT_SUPPORT_STRU                stImsSupport;

    /* IMS能力只有在FEATURE_IMS打开时，才有可能设置为开启 */
    /* 先获取NV的长度 */
    ulLength = 0;
    NV_GetLength(EN_NV_ID_IMS_RAT_SUPPORT, &ulLength);

    if (ulLength > sizeof(IMSA_NV_IMS_RAT_SUPPORT_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImsRatSupportNvim():WARNING: EN_NV_ID_IMS_RAT_SUPPORT length Error");

        return;
    }

    /* 读NV项EN_NV_ID_IMS_RAT_SUPPORT，失败，直接返回 */
    if (NV_OK != NV_Read(EN_NV_ID_IMS_RAT_SUPPORT,
                         &stImsSupport, ulLength))
    {

        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImsRatSupportNvim():WARNING: read EN_NV_ID_IMS_RAT_SUPPORT Error");

        return;
    }

    /* 赋值到全局变量中 */
    NAS_MML_SetLteImsSupportFlg(stImsSupport.ucLteImsSupportFlag);

    return;
}


VOS_VOID NAS_MMC_ReadImsCapNvim(VOS_VOID)
{
    VOS_UINT32                                  ulLength;
    IMS_NV_IMS_CAP_STRU                         stImsCapa;

    /* 先获取NV的长度 */
    ulLength = 0;
    NV_GetLength(EN_NV_ID_IMS_CAPABILITY, &ulLength);

    if (ulLength > sizeof(IMS_NV_IMS_CAP_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImsCapNvim():WARNING: EN_NV_ID_IMS_CAPABILITY length Error");

        return;
    }

    /* 读NV项EN_NV_ID_IMS_CAPABILITY，失败，直接返回 */
    if (NV_OK != NV_Read(EN_NV_ID_IMS_CAPABILITY,
                         &stImsCapa, ulLength))
    {

        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadImsCapNvim():WARNING: read EN_NV_ID_IMS_CAPABILITY Error");

        return;
    }

    /* 赋值到全局变量中 */
    NAS_MML_SetVoiceCallOnImsSupportFlag(stImsCapa.ucVoiceCallOnImsSupportFlag);
    NAS_MML_SetVideoCallOnImsSupportFlag(stImsCapa.ucVideoCallOnImsSupportFlag);
    NAS_MML_SetSmsOnImsSupportFlag(stImsCapa.ucSmsOnImsSupportFlag);

    return;
}
VOS_VOID NAS_MMC_ReadUssdOnImsNvim(VOS_VOID)
{
    VOS_UINT32                                  ulLength;
    TAF_NV_IMS_USSD_SUPPORT_STRU                stUssdFlg;

    /* 先获取NV的长度 */
    ulLength = 0;
    NV_GetLength(en_NV_Item_IMS_USSD_SUPPORT_FLG, &ulLength);

    if (ulLength > sizeof(TAF_NV_IMS_USSD_SUPPORT_STRU))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadUssdOnImsNvim():WARNING: en_NV_Item_IMS_USSD_SUPPORT_FLG length Error");

        return;
    }

    /* 读NV项en_NV_Item_IMS_USSD_SUPPORT_FLG，失败，直接返回 */
    if (NV_OK != NV_Read(en_NV_Item_IMS_USSD_SUPPORT_FLG,
                         &stUssdFlg, ulLength))
    {

        NAS_ERROR_LOG(WUEPS_PID_MMC,
                      "NAS_MMC_ReadUssdOnImsNvim():WARNING: read en_NV_Item_IMS_USSD_SUPPORT_FLG Error");

        return;
    }

    /* NV项激活，更新USSD支持配置信息 */
    NAS_MML_SetUssdOnImsSupportFlag(stUssdFlg.ucUssdOnImsSupportFlag);

    return;
}

#endif
VOS_VOID NAS_MMC_ReadProtectMtCsfbPagingProcedureLenNvim(VOS_VOID)
{
    NAS_MMC_NVIM_PROTECT_MT_CSFB_PAGING_PROCEDURE_LEN_STRU  stNvimProtectMtCsfbPagingProcedureLen;;

    PS_MEM_SET(&stNvimProtectMtCsfbPagingProcedureLen, 0x00, sizeof(stNvimProtectMtCsfbPagingProcedureLen));

    if (NV_OK != NV_Read(en_NV_Item_Protect_Mt_Csfb_Paging_Procedure_Len,
                        (VOS_VOID *)&stNvimProtectMtCsfbPagingProcedureLen,
                        sizeof(NAS_MMC_NVIM_PROTECT_MT_CSFB_PAGING_PROCEDURE_LEN_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC,
                     "NAS_MMC_ReadProtectMtCsfbPagingProcedureLenNvim():en_NV_Item_Protect_Mt_Csfb_Paging_Procedure_Len Error");
        return;
    }

    if (stNvimProtectMtCsfbPagingProcedureLen.usMtCsfbPagingProcedureLen < NAS_MML_PROTECT_MT_CSFB_PAGING_PROCEDURE_MIN_LEN)
    {
        NAS_MML_SetProtectMtCsfbPagingProcedureLen(NAS_MML_PROTECT_MT_CSFB_PAGING_PROCEDURE_MIN_LEN);

        return;
    }

    if (stNvimProtectMtCsfbPagingProcedureLen.usMtCsfbPagingProcedureLen > NAS_MML_PROTECT_MT_CSFB_PAGING_PROCEDURE_MAX_LEN)
    {
        NAS_MML_SetProtectMtCsfbPagingProcedureLen(NAS_MML_PROTECT_MT_CSFB_PAGING_PROCEDURE_MAX_LEN);

        return;
    }    

    NAS_MML_SetProtectMtCsfbPagingProcedureLen(stNvimProtectMtCsfbPagingProcedureLen.usMtCsfbPagingProcedureLen);
    
    return;
}

#if (FEATURE_ON == FEATURE_LTE)

VOS_VOID NAS_MMC_ReadLteOos2GPrefPlmnSelCfgNvim(VOS_VOID)
{
    VOS_UINT32                                              i;
    NAS_MML_SIM_FORMAT_PLMN_ID                              stSimPlmn;
    NAS_MML_PLMN_ID_STRU                                    stPlmn;
    NAS_MMC_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_CFG_STRU          stLteOosPlmnSelCfgNvim;
    NAS_MML_LTE_OOS_2G_PREF_PLMN_SEL_CFG_STRU              *pstLteOosPlmnSelCfg = VOS_NULL_PTR;

    PS_MEM_SET(&stLteOosPlmnSelCfgNvim, 0x00, sizeof(NAS_MMC_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_CFG_STRU));
    PS_MEM_SET(&stSimPlmn, 0x00, sizeof(stSimPlmn));
    PS_MEM_SET(&stPlmn, 0x00, sizeof(stPlmn));

    pstLteOosPlmnSelCfg = NAS_MML_GetLteOos2GPrefPlmnSelCfg();
    pstLteOosPlmnSelCfg->ucImsiListNum  = 0;
    
    if (NV_OK != NV_Read(en_NV_Item_LTE_OOS_2G_PREF_PLMN_SEL_CFG, &stLteOosPlmnSelCfgNvim, sizeof(NAS_MMC_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_CFG_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_ReadLteOos2GPrefPlmnSelCfgNvim:Read NV Failed");
        return;
    }

    /* 参数保护,超过最大值按照最大值计算 */
    if (NAS_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_MAX_IMSI_LIST_NUM < stLteOosPlmnSelCfgNvim.ucImsiListNum)
    {
        stLteOosPlmnSelCfgNvim.ucImsiListNum = NAS_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_MAX_IMSI_LIST_NUM;
    }

    /* 将USIM中保存的PLMN信息转换为内存中的信息 */
    for ( i = 0; i < stLteOosPlmnSelCfgNvim.ucImsiListNum; i++ )
    {
        PS_MEM_CPY(stSimPlmn.aucSimPlmn, stLteOosPlmnSelCfgNvim.astImsiList[i].aucSimPlmn, NAS_MML_SIM_PLMN_ID_LEN);
        NAS_MMC_ConvertSimPlmnToNasPLMN(&stSimPlmn, &stPlmn);

        if (VOS_TRUE == NAS_MML_IsPlmnIdValid(&stPlmn))
        {
            pstLteOosPlmnSelCfg->astImsiList[pstLteOosPlmnSelCfg->ucImsiListNum].ulMcc = stPlmn.ulMcc;
            pstLteOosPlmnSelCfg->astImsiList[pstLteOosPlmnSelCfg->ucImsiListNum].ulMnc = stPlmn.ulMnc;
            pstLteOosPlmnSelCfg->ucImsiListNum++;
        }
    }
}
#endif

/*lint -restore */


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


