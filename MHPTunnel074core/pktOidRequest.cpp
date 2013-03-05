/*
 * pktOidRequest.c - NDIS OID Request support module
 *
 * Copyright(c) 2005 Ryo Maruyama <ryo@jec.ac.jp> ALL RIGHTS RESERVED
 */

#include "pktOidRequest.h"


/*
 * pktGetGenLinkSpeed - get current interface link speed
 *
 *  ptrAdap     adapter handle
 *  ptrSpeed    result buffer
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktGetGenLinkSpeed(LPADAPTER ptrAdap, DWORD *ptrSpeed)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(ptrSpeed)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_GEN_LINK_SPEED;
    ptrOid->Length  = sizeof(DWORD);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrSpeed, ptrOid->Data, sizeof(DWORD));

    return TRUE;
}

/*
 * pktGetGenConnectStatus - get media connect status
 *
 *  ptrAdap     adapter handle
 *  ptrStat     result buffer
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktGetGenConnectStatus(LPADAPTER ptrAdap, PNDIS_MEDIA_STATE ptrStat)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_MEDIA_STATE)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_GEN_MEDIA_CONNECT_STATUS;
    ptrOid->Length  = sizeof(NDIS_MEDIA_STATE);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrStat, ptrOid->Data, sizeof(NDIS_MEDIA_STATE));

    return TRUE;
}

/*
 * pktGetGenPhysicalMedium - get a interface physical medium
 *
 *  ptrAdap     adapter handle
 *  ptrMedia    result buffer
 *  result      TRUE if successfly, FALSE then error 
 */
BOOLEAN
pktGetGenPhysicalMedium(LPADAPTER ptrAdap, PNDIS_PHYSICAL_MEDIUM ptrMedia)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_PHYSICAL_MEDIUM)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_GEN_PHYSICAL_MEDIUM;
    ptrOid->Length  = sizeof(NDIS_PHYSICAL_MEDIUM);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrMedia, ptrOid->Data, sizeof(NDIS_PHYSICAL_MEDIUM));

    return TRUE;
}

/*
 * pktGetGenVendorDesc - get vendor specificated desctiption
 *
 *  ptrAdap     adapter handle
 *  ptrDesc     result buffer
 *  max         max length for result buffer
 *  result      TRUE if successfly, FALSE then error
 */
BOOLEAN
pktGetGenVendorDesc(LPADAPTER ptrAdap, char *ptrDesc, int max)
{
    unsigned char       *buffer;
    PPACKET_OID_DATA    ptrOid;
    BOOLEAN             result = FALSE;

    buffer = (unsigned char*)calloc(sizeof(PACKET_OID_DATA) + max, 1);
    if ( NULL == buffer )
      goto fatal;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_GEN_VENDOR_DESCRIPTION;
    ptrOid->Length  = max;

    if (PacketRequest(ptrAdap, FALSE, ptrOid))
      {
        memcpy(ptrDesc, ptrOid->Data, max);
        result = TRUE;
      }

fatal:
    if ( NULL != buffer )
      free(buffer);
    return result;
}

/*
 * pktGet802_3MacAddress - get interface mac address
 *
 *  pktAdap     adapter handle
 *  ptrAddr     result buffer (length 6bytes)
 *  result      TRUE if succesfuly, FALSE then faild
 */
BOOLEAN
pktGet802_3MacAddress(LPADAPTER ptrAdap, UCHAR *ptrAddr)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + 6];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_3_PERMANENT_ADDRESS;
    ptrOid->Length  = 6;

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrAddr, ptrOid->Data, 6);

    return TRUE;
}

/*
 * pktGet802_11BSSID - get current BSSID
 *
 *  ptrAdap     adapter handle
 *  ptrBssid    result buffer
 *  result      TRUE if successfly, FALSE then error
 */
BOOLEAN
pktGet802_11BSSID(LPADAPTER ptrAdap, NDIS_802_11_MAC_ADDRESS ptrBssid)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_MAC_ADDRESS)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_BSSID;
    ptrOid->Length  = sizeof(NDIS_WLAN_BSSID);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrBssid, ptrOid->Data, sizeof(NDIS_802_11_MAC_ADDRESS));

    return TRUE;
}

/*
 * pktSet802_11BSSID - set BSSID
 *
 *  ptrAdap     adapter handle
 *  ptrBssid    pointer to new BSSID
 *  result      TRUE if successfly, FALSE then error
 */
BOOLEAN
pktSet802_11BSSID(LPADAPTER ptrAdap, NDIS_802_11_MAC_ADDRESS ptrBssid)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_MAC_ADDRESS)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_BSSID;
    ptrOid->Length  = sizeof(NDIS_WLAN_BSSID);

    memcpy(ptrOid->Data, ptrBssid, sizeof(NDIS_802_11_MAC_ADDRESS));

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktGet802_11SSID - get current SSID
 *
 *  ptrAdap     adapter handle
 *  ptrSsid     result pointer
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktGet802_11SSID(LPADAPTER ptrAdap, PNDIS_802_11_SSID ptrSsid)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_SSID)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_SSID;
    ptrOid->Length  = sizeof(NDIS_802_11_SSID);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrSsid, ptrOid->Data, sizeof(NDIS_802_11_SSID));

    return TRUE;
}

/*
 * pktSet802_11SSID - set current SSID
 *
 *  ptrAdap     adapter handle
 *  ptrSsid     pointer to new SSID structure
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktSet802_11SSID(LPADAPTER ptrAdap, PNDIS_802_11_SSID ptrSsid)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_SSID)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_SSID;
    ptrOid->Length  = sizeof(NDIS_802_11_SSID);

    memcpy(ptrOid->Data, ptrSsid, sizeof(NDIS_802_11_SSID));

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktGet802_11RSSI - get current RSSI(signal level)
 *
 *  ptrAdap     adapter handle
 *  ptrRssi     result pointer
 *  result      TRUE if successfly, FALSE then error
 */
BOOLEAN
pktGet802_11RSSI(LPADAPTER ptrAdap, NDIS_802_11_RSSI *ptrRssi)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_RSSI)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_RSSI;
    ptrOid->Length  = sizeof(NDIS_802_11_RSSI);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrRssi, ptrOid->Data, sizeof(NDIS_802_11_RSSI));

    return TRUE;
}

/*
 * pktGet802_11NetworkMode - get current network mode
 *
 *  ptrAdap     adapter handle
 *  ptrMode     result pointer
 *  result      TRUE if successfuly, false then error
 */
BOOLEAN
pktGet802_11NetworkMode(LPADAPTER ptrAdap, PNDIS_802_11_NETWORK_INFRASTRUCTURE ptrMode)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_INFRASTRUCTURE_MODE;
    ptrOid->Length  = sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrMode, ptrOid->Data, sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE));

    return TRUE;
}

/*
 * pktSet802_11NetworkMode - set network mode
 *
 *  ptrAdap     adapter handle
 *  mode        new network mode value
 *  result      TRUE if successfuly, false then error
 */
BOOLEAN
pktSet802_11NetworkMode(LPADAPTER ptrAdap, NDIS_802_11_NETWORK_INFRASTRUCTURE mode)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_INFRASTRUCTURE_MODE;
    ptrOid->Length  = sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE);

    memcpy(ptrOid->Data, &mode, sizeof(NDIS_802_11_NETWORK_INFRASTRUCTURE));

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktGet802_11Configuration - get current interface configuration
 *
 *  ptrAdap     adapter handle
 *  ptrCfg      result pointer
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktGet802_11Configuration(LPADAPTER ptrAdap, PNDIS_802_11_CONFIGURATION ptrCfg)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_CONFIGURATION)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_CONFIGURATION;
    ptrOid->Length  = sizeof(NDIS_802_11_CONFIGURATION);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrCfg, ptrOid->Data, sizeof(NDIS_802_11_CONFIGURATION));

    return TRUE;
}

/*
 * pktSet802_11Configuration - set interface configuration
 *
 *  ptrAdap     adapter handle
 *  ptrCfg      pointer to new configuration structure
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktSet802_11Configuration(LPADAPTER ptrAdap, PNDIS_802_11_CONFIGURATION ptrCfg)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_CONFIGURATION)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_CONFIGURATION;
    ptrOid->Length  = sizeof(NDIS_802_11_CONFIGURATION);

    memcpy(ptrOid->Data, ptrCfg, sizeof(NDIS_802_11_CONFIGURATION));

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktGet802_11Statistics - get current interface statistics
 *
 *  ptrAdap     adapter handle
 *  ptrStat     result pointer
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktGet802_11Statistics(LPADAPTER ptrAdap, PNDIS_802_11_STATISTICS ptrStat)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(PNDIS_802_11_STATISTICS)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_STATISTICS;
    ptrOid->Length  = sizeof(NDIS_802_11_STATISTICS);

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    memcpy(ptrStat, ptrOid->Data, sizeof(NDIS_802_11_STATISTICS));

    return TRUE;
}

/*
 * pktExec802_11AddWEP - add new wep key
 *
 *  ptrAdapter      adapter handle
 *  ptrKey          pointer to new wep key
 *  result          TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktExec802_11AddWEP(LPADAPTER ptrAdap, PNDIS_802_11_WEP ptrKey)
{
    unsigned char       *buffer;
    PPACKET_OID_DATA    ptrOid;
    BOOLEAN             result = FALSE;

    buffer = (unsigned char*)calloc(sizeof(PACKET_OID_DATA) + ptrKey->Length, 1);
    if (NULL == buffer )
      goto fatal;

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_ADD_WEP;
    ptrOid->Length  = ptrKey->Length;

    memcpy(ptrOid->Data, ptrKey, ptrKey->Length);

    if (PacketRequest(ptrAdap, TRUE, ptrOid))
      result = TRUE;

fatal:
    if (NULL != buffer)
      free(buffer);
    return result;
}

/*
 * pktExec802_11RemoveWEP - remove wep key
 *
 *  ptrAdap     adapter handle
 *  index       index for target wep key
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktExec802_11RemoveWEP(LPADAPTER ptrAdap, NDIS_802_11_KEY_INDEX index)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_KEY_INDEX)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_REMOVE_WEP;
    ptrOid->Length      = sizeof(NDIS_802_11_KEY_INDEX);

    memcpy(ptrOid->Data, &index, sizeof(NDIS_802_11_KEY_INDEX));

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktExec802_11Disassociate - disassociate from current network
 *
 *  ptrAdap     adapter handle
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktExec802_11Disassociate(LPADAPTER ptrAdap)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + 1024];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_DISASSOCIATE;
    ptrOid->Length      = 1024;

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktGet802_11BSSIDList - get a scaned BSSID list
 *
 *  ptrAdap     adapter handle
 *  ptrList     pointer to result array
 *  max         ptrList array length
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktGet802_11BSSIDList(LPADAPTER ptrAdap, PNDIS_WLAN_BSSID ptrList, int max)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + 4096];
    PPACKET_OID_DATA    ptrOid;
    PNDIS_802_11_BSSID_LIST ptrBssids;
    PNDIS_WLAN_BSSID    ptrBssid;
    ULONG               i;

    memset(buffer, 0, sizeof(buffer));

    ptrOid          = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid     = OID_802_11_BSSID_LIST;
    ptrOid->Length  = 1024;
    ptrBssids       = (PNDIS_802_11_BSSID_LIST)ptrOid->Data;

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    if ( 0 == ptrBssids->NumberOfItems )
      return TRUE;

    memset(ptrList, 0, sizeof(NDIS_WLAN_BSSID) * max);

    for ( i = 0, ptrBssid = ptrBssids->Bssid; i < ptrBssids->NumberOfItems; i++ )
      {
        memcpy(&ptrList[i], ptrBssid, sizeof(NDIS_WLAN_BSSID));

        /* figure structure length */
        ptrList[i].Length = sizeof(NDIS_WLAN_BSSID);

        ptrBssid = (PNDIS_WLAN_BSSID)((unsigned char*)ptrBssid + ptrBssid->Length);
      }

    return TRUE;
}

/*
 * pktGet802_11AuthMode - get current authentication mode
 *
 *  ptrAdap     adapter handle
 *  ptrMode     pointer to result pointer
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktGet802_11AuthMode(LPADAPTER ptrAdap, PNDIS_802_11_AUTHENTICATION_MODE ptrMode)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_AUTHENTICATION_MODE)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_AUTHENTICATION_MODE;
    ptrOid->Length      = sizeof(NDIS_802_11_AUTHENTICATION_MODE);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrMode, ptrOid->Data, sizeof(NDIS_802_11_AUTHENTICATION_MODE));

    return TRUE;
}

/*
 * pktSet802_11AuthMode - set authentication mode
 *
 *  ptrAdap     adapter handle
 *  mode     pointer to new authentication mode
 *  result      TRUE if successfuly, FALSE then error
 */
BOOLEAN
pktSet802_11AuthMode(LPADAPTER ptrAdap, NDIS_802_11_AUTHENTICATION_MODE mode)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_AUTHENTICATION_MODE)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_AUTHENTICATION_MODE;
    ptrOid->Length      = sizeof(NDIS_802_11_AUTHENTICATION_MODE);

    memcpy(ptrOid->Data, &mode, sizeof(NDIS_802_11_AUTHENTICATION_MODE));

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktExec802_11BSSIDScan - execute BSSID scan (non blocking)
 *
 *  ptrAdap     adapter handle
 *  result      TRUE if successfly, FALSE then error
 */
BOOLEAN
pktExec802_11BSSIDScan(LPADAPTER ptrAdap)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + 1024];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_BSSID_LIST_SCAN;
    ptrOid->Length      = 1;

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * pktGet802_11WEPStatus - get current WEP status
 *
 *  ptrAdap     adapter handle
 *  ptrStat     result pointer
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktGet802_11WEPStatus(LPADAPTER ptrAdap, PNDIS_802_11_WEP_STATUS ptrStat)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_WEP_STATUS)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_WEP_STATUS;
    ptrOid->Length      = sizeof(NDIS_802_11_WEP_STATUS);

    if (!PacketRequest(ptrAdap, FALSE, ptrOid))
      return FALSE;

    memcpy(ptrStat, ptrOid->Data, sizeof(NDIS_802_11_WEP_STATUS));

    return TRUE;
}

/*
 * pktSet802_11WEPStatus - set WEP status
 *
 *  ptrAdap     adapter handle
 *  stat        new WEP status value
 *  result      TRUE if successfly, FALSE then faild
 */
BOOLEAN
pktSet802_11WEPStatus(LPADAPTER ptrAdap, NDIS_802_11_WEP_STATUS stat)
{
    unsigned char       buffer[sizeof(PACKET_OID_DATA) + sizeof(NDIS_802_11_WEP_STATUS)];
    PPACKET_OID_DATA    ptrOid;

    ptrOid              = (PPACKET_OID_DATA)buffer;
    ptrOid->Oid         = OID_802_11_WEP_STATUS;
    ptrOid->Length      = sizeof(NDIS_802_11_WEP_STATUS);

    memcpy(ptrOid->Data, &stat, sizeof(NDIS_802_11_WEP_STATUS));

    if (!PacketRequest(ptrAdap, TRUE, ptrOid))
      return FALSE;

    return TRUE;
}

/*
 * EOF
 */
