/*
* $Id: dsstypes.h,v 1.3 2005/10/28 02:57:04 jms Exp $
*
* Revision History
* ===================
* $Log: dsstypes.h,v $
* Revision 1.3  2005/10/28 02:57:04  jms
* allow for larger names in customer table
*
* Revision 1.2  2005/01/03 20:08:58  jms
* change line terminations
*
* Revision 1.1.1.1  2004/11/24 23:31:46  jms
* re-establish external server
*
* Revision 1.3  2004/04/07 20:17:29  jms
* bug #58 (join fails between order/lineitem)
*
* Revision 1.2  2004/01/22 05:49:29  jms
* AIX porting (AIX 5.1)
*
* Revision 1.1.1.1  2003/08/07 17:58:34  jms
* recreation after CVS crash
*
* Revision 1.2  2003/08/07 17:58:34  jms
* Convery RNG to 64bit space as preparation for new large scale RNG
*
* Revision 1.1.1.1  2003/04/03 18:54:21  jms
* initial checkin
*
*
*/
 /* 
 * general definitions and control information for the DSS data types
 * and function prototypes
 */

/*
 * typedefs
 */
typedef struct
{
    DSS_HUGE        custkey;
    char            name[C_NAME_LEN + 3];
    int             nlen;
    char            address[C_ADDR_MAX + 1];
    int             alen;
    char            city[CITY_FIX+1];
    int             nation_key;
    char            nation_name[C_NATION_NAME_LEN+1];
    int             region_key;
    char            region_name[C_REGION_NAME_LEN+1];
    char            phone[PHONE_LEN + 1];
    char            mktsegment[MAXAGG_LEN + 1];
}               customer_t;
/* customers.c */
long mk_cust   PROTO((DSS_HUGE n_cust, customer_t * c));
int pr_cust    PROTO((customer_t * c, int mode));
int ld_cust    PROTO((customer_t * c, int mode));

typedef struct
{
   long            datekey;
   char            date[D_DATE_LEN+1];
   char            dayofweek[D_DAYWEEK_LEN+1] ;
   char            month[D_MONTH_LEN+1];
   long            year;
   long            yearmonthnum;
   char            yearmonth[D_YEARMONTH_LEN+1];
   long            daynuminweek;
   long            daynuminmonth;
   long            daynuminyear;
   long            monthnuminyear;
   long            weeknuminyear;
   char            sellingseason[D_SEASON_LEN + 1];
   int             slen;
   char            lastdayinweekfl[2];
   char            lastdayinmonthfl[2];
   char            holidayfl[2];
   char            weekdayfl[2];
}               date_t;
/* date.c */
long mk_date   PROTO((DSS_HUGE index, date_t * d));
int pr_date    PROTO((date_t * date, int mode));
int ld_date    PROTO((date_t * date, int mode));

typedef struct
{
    DSS_HUGE	    okey;  /*for clustering line items*/
    DSS_HUGE        linenumber; /*integer, constrain to max of 7*/
    DSS_HUGE        custkey;
    DSS_HUGE        partkey;
    DSS_HUGE        suppkey;
    char            orderdate[DATE_LEN];
    char            opriority[MAXAGG_LEN + 1];
    long            ship_priority;
    DSS_HUGE        quantity;
    long            extended_price;
    long            order_totalprice;
    DSS_HUGE        discount;
    long            revenue;
    long            supp_cost;
    DSS_HUGE        tax;
    char            commit_date[DATE_LEN] ;
    char            shipmode[O_SHIP_MODE_LEN + 1];
}               lineorder_t;

typedef struct
{
    DSS_HUGE	    okey;
    DSS_HUGE        custkey;
    DSS_HUGE        totalprice;
    char            odate[DATE_LEN];
    char            opriority[MAXAGG_LEN + 1];
    char            clerk[O_CLRK_LEN + 1];
    long            spriority;
    DSS_HUGE        lines;
    lineorder_t     lineorders[O_LCNT_MAX];
}   order_t;

/* lineorder.c */
long mk_order   PROTO((DSS_HUGE index, order_t * o, long upd_num));
int pr_order    PROTO((order_t * o, int mode));
int ld_order    PROTO((order_t * o, int mode));

typedef struct
{
    DSS_HUGE       partkey;
    char           name[P_NAME_LEN + 1];
    int            nlen;
    char           mfgr[P_MFG_LEN + 1];
    char           category[P_CAT_LEN + 1];
    char           brand[P_BRND_LEN + 1];
    char           color[P_COLOR_MAX + 1];
    int            clen;
    char           type[P_TYPE_MAX + 1];
    int            tlen;
    DSS_HUGE       size;
    char           container[P_CNTR_LEN + 1];
}               part_t;

/* parts.c */
long mk_part   PROTO((DSS_HUGE index, part_t * p));
int pr_part    PROTO((part_t * part, int mode));
int ld_part    PROTO((part_t * part, int mode));

typedef struct
{
    DSS_HUGE        suppkey;
    char            name[S_NAME_LEN + 1];
    char            address[S_ADDR_MAX + 1];
    int             alen; 
    char            city[CITY_FIX +1];
    DSS_HUGE        nation_key;
    char            nation_name[S_NATION_NAME_LEN+1];
    DSS_HUGE        region_key;
    char            region_name[S_REGION_NAME_LEN+1];
    char            phone[PHONE_LEN + 1];
}               supplier_t;
/* supplier.c */
long mk_supp   PROTO((DSS_HUGE index, supplier_t * s));
int pr_supp    PROTO((supplier_t * supp, int mode));
int ld_supp    PROTO((supplier_t * supp, int mode));

typedef struct
{
    DSS_HUGE            timekey;
    char            alpha[DATE_LEN];
    long            year;
    long            month;
    long            week;
    long            day;
} dss_time_t;               

/* time.c */
long mk_time   PROTO((DSS_HUGE h, dss_time_t * t));
