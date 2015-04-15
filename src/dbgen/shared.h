/*
* $Id: shared.h,v 1.2 2005/01/03 20:08:59 jms Exp $
*
* Revision History
* ===================
* $Log: shared.h,v $
* Revision 1.2  2005/01/03 20:08:59  jms
* change line terminations
*
* Revision 1.1.1.1  2004/11/24 23:31:47  jms
* re-establish external server
*
* Revision 1.1.1.1  2003/04/03 18:54:21  jms
* recreation after CVS crash
*
* Revision 1.1.1.1  2003/04/03 18:54:21  jms
* initial checkin
*
*
*/
#define N_CMNT_LEN      72
#define N_CMNT_MAX      152
#define R_CMNT_LEN      72
#define R_CMNT_MAX      152
#define  MONEY_SCL     0.01
#define  V_STR_HGH    1.6
#define  P_NAME_LEN    55
#define  P_MFG_LEN     25
#define  P_BRND_LEN    10
#define  P_TYPE_LEN    25
#define  P_CNTR_LEN    10
#define  P_CMNT_LEN    14
#define  P_CMNT_MAX    23
#define  P_CAT_LEN     7
#define  P_COLOR_MAX   11
#define  P_TYPE_MAX    25
#define  P_CAT_MIN     1
#define  P_CAT_MAX     5
#define  P_CAT_SD      97
#define  P_COLOR_LEN   3
#define  S_NAME_LEN    25
#define  S_ADDR_LEN    25
#define  S_ADDR_MAX    40
#define  S_CMNT_LEN    63
#define  S_CMNT_MAX   101
#define  S_NATION_NAME_LEN 15
#define  S_REGION_NAME_LEN 12
#define  C_NAME_LEN    18
#define  C_ADDR_LEN    25
#define  C_ADDR_MAX    40
#define  C_MSEG_LEN    10
#define  C_CMNT_LEN    73
#define  C_CMNT_MAX    117
#define  C_NATION_NAME_LEN 15
#define  C_REGION_NAME_LEN 12
#define  L_CMNT_LEN    27
#define  L_CMNT_MAX    44
#define  L_INST_LEN    25
#define  L_SMODE_LEN   10
#define  T_ALPHA_LEN   10
#define  DATE_LEN      13  /* long enough to hold either date format */
#define  PHONE_LEN     15
#define  MAXAGG_LEN    20    /* max component length for a agg str */
#define  P_CMNT_SD      6
#define  PS_CMNT_SD     9
#define  O_CMNT_SD     12
#define  O_SHIP_MODE_LEN 10
#define  O_CLRK_LEN    15
#define  O_OPRIO_LEN   8
#define  C_ADDR_SD     26
#define  C_CMNT_SD     31
#define  S_ADDR_SD     32
#define  S_CMNT_SD     36
#define  L_CMNT_SD     25
#define  L_SKEY_MIN   1
#define  L_SKEY_MAX (tdefs[SUPP].base * scale)
#define  D_DATE_LEN 18
#define  D_DAYWEEK_LEN 9
#define  D_MONTH_LEN  9
#define  D_YEARMONTH_LEN 7
#define  D_SEASON_LEN 12
#define  D_STARTDATE 694245661 /*corresponding to 1/1/1992 1:1:1*/

#define NUM_HOLIDAYS 10
#define NUM_SEASONS 5

#define CITY_FIX 10
