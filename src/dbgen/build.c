/*
 * $Id: build.c,v 1.5 2009/06/28 14:01:08 jms Exp $
 * 
 * Revision History =================== $Log: build.c,v $
 * Revision History =================== Revision 1.5  2009/06/28 14:01:08  jms
 * Revision History =================== bug fix for DOP
 * Revision History =================== Revision 1.4
 * 2005/10/28 02:56:22  jms add platform-specific printf formats to allow for
 * DSS_HUGE data type
 * 
 * Revision 1.3  2005/10/14 23:16:54  jms fix for answer set compliance
 * 
 * Revision 1.2  2005/01/03 20:08:58  jms change line terminations
 * 
 * Revision 1.1.1.1  2004/11/24 23:31:46  jms re-establish external server
 * 
 * Revision 1.3  2004/04/07 20:17:29  jms bug #58 (join fails between
 * order/lineitem)
 * 
 * Revision 1.2  2004/01/22 05:49:29  jms AIX porting (AIX 5.1)
 * 
 * Revision 1.1.1.1  2003/08/08 21:35:26  jms recreation after CVS crash
 * 
 * Revision 1.3  2003/08/08 21:35:26  jms first integration of rng64 for
 * o_custkey and l_partkey
 * 
 * Revision 1.2  2003/08/07 17:58:34  jms Convery RNG to 64bit space as
 * preparation for new large scale RNG
 * 
 * Revision 1.1.1.1  2003/04/03 18:54:21  jms initial checkin
 * 
 * 
 */
/* stuff related to the customer table */
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef VMS
#include <sys/types.h>
#endif
#if defined(SUN)
#include <unistd.h>
#endif
#include <math.h>

#include "dss.h"
#include "dsstypes.h"
#ifdef ADHOC
#include "adhoc.h"
extern adhoc_t  adhocs[];
#endif				/* ADHOC */
#include "rng64.h"

#define LEAP_ADJ(yr, mnth)      \
((LEAP(yr) && (mnth) >= 2) ? 1 : 0)
#define JDAY_BASE       8035	/* start from 1/1/70 a la unix */
#define JMNTH_BASE      (-70 * 12)	/* start from 1/1/70 a la unix */
#define JDAY(date) ((date) - STARTDATE + JDAY_BASE + 1)
#define PART_SUPP_BRIDGE(tgt, p, s) \
    { \
    DSS_HUGE tot_scnt = tdefs[SUPP].base * scale; \
    tgt = (p + s *  (tot_scnt / SUPP_PER_PART +  \
	(long) ((p - 1) / tot_scnt))) % tot_scnt + 1; \
    }
#define V_STR(avg, sd, tgt)  a_rnd((int)(avg * V_STR_LOW),(int)(avg * V_STR_HGH), sd, tgt)
#define TEXT(avg, sd, tgt)  dbg_text(tgt, (int)(avg * V_STR_LOW),(int)(avg * V_STR_HGH), sd)
int gen_city PROTO((char *cityName, char *nationName));
int gen_color PROTO((char * source, char * dest));
static void gen_phone PROTO((DSS_HUGE ind, char *target, long seed));

int gen_holiday_fl PROTO((char * dest, int month, int day));
int is_last_day_in_month PROTO((int year,int month,int day));
int gen_season PROTO((char * dest,int month,int day));

/*Following functions are related to date table generation*/
int days_in_a_month[12]={31,28,31,30,31,30,31,31,30,31,30,31};
int days_in_a_month_l[12]={31,29,31,30,31,30,31,31,30,31,30,31};
season seasons[]={
  {"Christmas",1,11,31,12},
  {"Summer",1,5,31,8},
  {"Winter",1,1,31,3},
  {"Spring",1,4,30,4},
  {"Fall",1,9,31,10}
};
holiday holidays[]={
  {"Christmas",12,24},
  {"New Years Day", 1,1},
  {"holiday1", 2,20},
  {"Easter Day",4,20},
  {"holiday2", 5,20},
  {"holiday3",7,20},
  {"holiday4",8,20},
  {"holiday5",9,20},
  {"holiday6",10,20},
  {"holiday7",11,20}
};

char * month_names[]={"January","February","March","April",
                 "May","June","July","Augest",
                 "September","Octorber","November","December"};

char * weekday_names[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

/*make the date table, it takes the continuous index , and add index*60*60*24 to 
 *numeric representation 1/1/1992 01:01:01, 
 *then convert the final numeric date time to tm structure, and thus extract other field
 *for date_t structure */
long
mk_date(DSS_HUGE index,date_t *d)
{
    long espan = (index-1)*60*60*24;
    
    time_t numDateTime = D_STARTDATE + espan; 

    struct tm *localTime = localtime(&numDateTime); 
 
    /*make Sunday be the first day of a week */
    d->daynuminweek=((long)localTime->tm_wday+1)%7+1;
    d->monthnuminyear=(long)localTime->tm_mon+1;
    strncpy(d->dayofweek, weekday_names[d->daynuminweek-1],D_DAYWEEK_LEN+1);
    strncpy(d->month,month_names[d->monthnuminyear-1],D_MONTH_LEN+1);
    d->year=(long)localTime->tm_year + 1900;
    d->daynuminmonth=(long)localTime->tm_mday;
    d->yearmonthnum=d->year * 100 + d->monthnuminyear;

    sprintf(d->yearmonth,"%.3s%ld",d->month,d->year);
    sprintf(d->date,"%s %ld, %ld",d->month,d->daynuminmonth,d->year);
    
    d->datekey = d->year*10000+d->monthnuminyear*100+ d->daynuminmonth; 

    d->daynuminyear=(int)localTime->tm_yday+1;
    d->weeknuminyear = d->daynuminyear/7 + 1;
    
    if(d->daynuminweek ==7){
	d->lastdayinweekfl[0]='1';
    }
    else{
	d->lastdayinweekfl[0]='0';
    }
    d->lastdayinweekfl[1]='\0';

    if(is_last_day_in_month(d->year,d->monthnuminyear,d->daynuminmonth)==1){
	d->lastdayinmonthfl[0]= '0';
    }else{
	d->lastdayinmonthfl[0]= '1';
    }
    d->lastdayinmonthfl[1]='\0';
 
    if(d->daynuminweek!=1 && d->daynuminweek!=7){
	d->weekdayfl[0]='1';
    }
    else{
	d->weekdayfl[0]='0';
    }
    
    d->weekdayfl[1]='\0';

    gen_season(d->sellingseason,d->monthnuminyear,d->daynuminmonth);
    d->slen = strlen(d->sellingseason);
    gen_holiday_fl(d->holidayfl,d->monthnuminyear,d->daynuminmonth);    
    return (0);
}

DSS_HUGE
rpb_routine(DSS_HUGE p)
{
	DSS_HUGE        price;

	price = 90000;
	price += (p / 10) % 20001;	/* limit contribution to $200 */
	price += (p % 1000) * 100;

	return (price);
}

static void
gen_phone(DSS_HUGE ind, char *target, long seed)
{
	DSS_HUGE        acode, exchg, number;

	RANDOM(acode, 100, 999, seed);
	RANDOM(exchg, 100, 999, seed);
	RANDOM(number, 1000, 9999, seed);

	sprintf(target, "%02d", (int) (10 + (ind % NATIONS_MAX)));
	sprintf(target + 3, "%03d", (int) acode);
	sprintf(target + 7, "%03d", (int) exchg);
	sprintf(target + 11, "%04d", (int) number);
	target[2] = target[6] = target[10] = '-';

	return;
}



long
mk_cust(DSS_HUGE n_cust, customer_t * c)
{
	DSS_HUGE        i;
	static int      bInit = 0;
	static char     szFormat[100];

	if (!bInit)
	{
		sprintf(szFormat, C_NAME_FMT, 9, HUGE_FORMAT + 1);
		bInit = 1;
	}
	c->custkey = n_cust;
	sprintf(c->name, szFormat, C_NAME_TAG, n_cust);
	V_STR(C_ADDR_LEN, C_ADDR_SD, c->address);
	c->alen = (int)strlen(c->address);
	RANDOM(i, 0, (nations.count - 1), C_NTRG_SD);
	strcpy(c->nation_name,nations.list[i].text);
	strcpy(c->region_name,regions.list[nations.list[i].weight].text);
	gen_city(c->city,c->nation_name);
	gen_phone(i, c->phone, (long) C_PHNE_SD);
	pick_str(&c_mseg_set, C_MSEG_SD, c->mktsegment);

	return (0);
}


/*
 * generate the numbered order and its associated lineitems
 */
void
mk_sparse(DSS_HUGE i, DSS_HUGE * ok, long seq)
{
	long            low_bits;

	*ok = i;
	low_bits = (long) (i & ((1 << SPARSE_KEEP) - 1));
	*ok = *ok >> SPARSE_KEEP;
	*ok = *ok << SPARSE_BITS;
	*ok += seq;
	*ok = *ok << SPARSE_KEEP;
	*ok += low_bits;


	return;
}

long
mk_order(DSS_HUGE index, order_t *o, long upd_num)
	{
	DSS_HUGE      lcnt;
	DSS_HUGE      rprice;
	long      ocnt;
	DSS_HUGE      tmp_date;
	DSS_HUGE      c_date;
	DSS_HUGE      clk_num;
	DSS_HUGE      supp_num;
	static char **asc_date = NULL;
	char tmp_str[2];
	char **mk_ascdate PROTO((void));
	int delta = 1;
	static int      bInit = 0;
    static char     szFormat[100];

	if (!bInit)
    {
        sprintf(szFormat, O_CLRK_FMT, 9, HUGE_FORMAT + 1);
        bInit = 1;
    }
	if (asc_date == NULL)
	    asc_date = mk_ascdate();

	RANDOM(tmp_date, O_ODATE_MIN, O_ODATE_MAX, O_ODATE_SD);
	strcpy(o->odate, asc_date[tmp_date - STARTDATE]);

	mk_sparse (index, &o->okey,
		(upd_num == 0) ? 0 : 1 + upd_num / (10000 / refresh));
	if (scale >= 30000)
        RANDOM64(o->custkey, O_CKEY_MIN, O_CKEY_MAX, O_CKEY_SD);
    else
        RANDOM(o->custkey, O_CKEY_MIN, O_CKEY_MAX, O_CKEY_SD);
	while (o->custkey % CUST_MORTALITY == 0)
	    {
		o->custkey += delta;
		o->custkey = MIN(o->custkey, O_CKEY_MAX);
		delta *= -1;
	    }
	pick_str(&o_priority_set, O_PRIO_SD, o->opriority);
	RANDOM(clk_num, 1, MAX((scale * O_CLRK_SCL), O_CLRK_SCL), O_CLRK_SD);
	o->spriority = 0;
	
	o->totalprice = 0;
	ocnt = 0;
	
	RANDOM(o->lines, O_LCNT_MIN, O_LCNT_MAX, O_LCNT_SD);
	for (lcnt = 0; lcnt < o->lines; lcnt++)
	    {
		
		o->lineorders[lcnt].okey = o->okey;;
		o->lineorders[lcnt].linenumber = lcnt + 1;
		o->lineorders[lcnt].custkey = o->custkey;
		RANDOM(o->lineorders[lcnt].suppkey, L_SKEY_MIN, L_SKEY_MAX, L_SKEY_SD);
				
		RANDOM(o->lineorders[lcnt].quantity, L_QTY_MIN, L_QTY_MAX, L_QTY_SD);
		RANDOM(o->lineorders[lcnt].discount, L_DCNT_MIN, L_DCNT_MAX, L_DCNT_SD);
		RANDOM(o->lineorders[lcnt].tax, L_TAX_MIN, L_TAX_MAX, L_TAX_SD);

		strcpy(o->lineorders[lcnt].orderdate,o->odate);

		strcpy(o->lineorders[lcnt].opriority,o->opriority);
		o->lineorders[lcnt].ship_priority = o->spriority;

		RANDOM(c_date, L_CDTE_MIN, L_CDTE_MAX, L_CDTE_SD);
		c_date += tmp_date;        
		strcpy(o->lineorders[lcnt].commit_date, asc_date[c_date - STARTDATE]);

		pick_str(&l_smode_set, L_SMODE_SD, o->lineorders[lcnt].shipmode);
		
		if (scale >= 30000)
            RANDOM64(o->lineorders[lcnt].partkey, L_PKEY_MIN, L_PKEY_MAX, L_PKEY_SD);
        else
            RANDOM(o->lineorders[lcnt].partkey, L_PKEY_MIN, L_PKEY_MAX, L_PKEY_SD);
		rprice = rpb_routine(o->lineorders[lcnt].partkey);
		o->lineorders[lcnt].extended_price = rprice * o->lineorders[lcnt].quantity;
		o->lineorders[lcnt].revenue = o->lineorders[lcnt].extended_price * ((long)100-o->lineorders[lcnt].discount)/(long)PENNIES;
		
		//round off problem with linux if use 0.6
		o->lineorders[lcnt].supp_cost = 6 * rprice /10;
		
		o->totalprice +=
		    ((o->lineorders[lcnt].extended_price * 
		      ((long)100 - o->lineorders[lcnt].discount)) / (long)PENNIES ) *
		    ((long)100 + o->lineorders[lcnt].tax)
		    / (long)PENNIES;
	    }
	
	for (lcnt = 0; lcnt < o->lines; lcnt++)
	    {
		o->lineorders[lcnt].order_totalprice = o->totalprice;
	    }
	return (0);
	}

long
mk_part(DSS_HUGE index, part_t * p)
{
	DSS_HUGE        temp;
	long            snum;
	DSS_HUGE        brnd;
	static int      bInit = 0;
	static char     szFormat[100];
	static char     szBrandFormat[100];
	DSS_HUGE        mfgr;
	DSS_HUGE        cat;

	if (!bInit)
	{
		sprintf(szFormat, P_MFG_FMT, 1, HUGE_FORMAT + 1);
		sprintf(szBrandFormat, P_BRND_FMT, 2, HUGE_FORMAT + 1);
		bInit = 1;
	}
	p->partkey = index;
	
	agg_str(&colors, (long) P_NAME_SCL, (long) P_NAME_SD, p->name);
	
	/*extract color from substring of p->name*/
    p->clen =gen_color(p->name,p->color); 
	
	
	RANDOM(mfgr, P_MFG_MIN, P_MFG_MAX, P_MFG_SD);
	sprintf(p->mfgr, "%s%lld", "MFGR#", mfgr);

    RANDOM(cat, P_CAT_MIN, P_CAT_MAX, P_CAT_SD);
    sprintf(p->category, "%s%lld", p->mfgr,cat);
	

	RANDOM(brnd, P_BRND_MIN, P_BRND_MAX, P_BRND_SD);
	sprintf(p->brand,"%s%lld",p->category,brnd);

	p->tlen = pick_str(&p_types_set, P_TYPE_SD, p->type);
	p->tlen = strlen(p_types_set.list[p->tlen].text);
	RANDOM(p->size, P_SIZE_MIN, P_SIZE_MAX, P_SIZE_SD);
	
	pick_str(&p_cntr_set, P_CNTR_SD, p->container);
	return (0);
}

long
mk_supp(DSS_HUGE index, supplier_t * s)
{
	DSS_HUGE        i, bad_press, noise, offset, type;
	static int      bInit = 0;
	static char     szFormat[100];

	if (!bInit)
	{
		sprintf(szFormat, S_NAME_FMT, 9, HUGE_FORMAT + 1);
		bInit = 1;
	}
	s->suppkey = index;
	sprintf(s->name, szFormat, S_NAME_TAG, index);
	V_STR(S_ADDR_LEN, S_ADDR_SD, s->address);
	s->alen = (int)strlen(s->address);
	RANDOM(i, 0, nations.count - 1, S_NTRG_SD);
	strcpy(s->nation_name,nations.list[i].text);
    strcpy(s->region_name,regions.list[nations.list[i].weight].text);
	gen_city(s->city,s->nation_name);
	gen_phone(i, s->phone, S_PHNE_SD);
	return (0);
}

struct
{
	char           *mdes;
	long            days;
	long            dcnt;
}               months[] =

{
	{
		NULL, 0, 0
	},
	{
		"JAN", 31, 31
	},
	{
		"FEB", 28, 59
	},
	{
		"MAR", 31, 90
	},
	{
		"APR", 30, 120
	},
	{
		"MAY", 31, 151
	},
	{
		"JUN", 30, 181
	},
	{
		"JUL", 31, 212
	},
	{
		"AUG", 31, 243
	},
	{
		"SEP", 30, 273
	},
	{
		"OCT", 31, 304
	},
	{
		"NOV", 30, 334
	},
	{
		"DEC", 31, 365
	}
};

long
mk_time(DSS_HUGE index, dss_time_t * t)
{
	long            m = 0;
	long            y;
	long            d;

	t->timekey = index + JDAY_BASE;
	y = julian(index + STARTDATE - 1) / 1000;
	d = julian(index + STARTDATE - 1) % 1000;
	while (d > months[m].dcnt + LEAP_ADJ(y, m))
		m++;
	PR_DATE(t->alpha, y, m,
		d - months[m - 1].dcnt - ((LEAP(y) && m > 2) ? 1 : 0));
	t->year = 1900 + y;
	t->month = m + 12 * y + JMNTH_BASE;
	t->week = (d + T_START_DAY - 1) / 7 + 1;
	t->day = d - months[m - 1].dcnt - LEAP_ADJ(y, m - 1);

	return (0);
}

int gen_city(char *cityName, char *nationName){
    int i=0;
    DSS_HUGE randomPick;
	int clen = strlen(cityName);
	int nlen = strlen(nationName);

    strncpy(cityName,nationName,CITY_FIX-1);
    
    if(nlen < CITY_FIX-1){
      for(i = nlen ; i< CITY_FIX-1;i++)
        cityName[i] = ' ';
    }
    RANDOM(randomPick, 0, 9, 98);
    
    sprintf(cityName+CITY_FIX-1,"%lld",randomPick);
    cityName[CITY_FIX] = '\0';
    return 0; 
}

/*
P_NAME is as long as 55 bytes in TPC-H, which is un¬reasonably large. 
We reduce it to 22 by limiting to a concatena¬tion of two colors (see [TPC-H], pg 94). 
We also add a new column named P_COLOR that could be used in queries where currently a 
color must be chosen by substring from P_NAME.
*/
int gen_color(char * source, char * dest){
  int i = 0,j=0;
  int clen=0;

  while(source[i]!= ' '  ){
      dest[i]=source[i];
      i++;
  }
  dest[i]='\0';

  i++;
  while(source[i] != '\0'){
	source[j] = source[i];
	j++;
	i++;
  }

  source[j] = '\0';
  
  clen = strlen(dest);
  return clen;
}

int gen_holiday_fl(char * dest, int month, int day){
  int i;
  for(i = 0; i< NUM_HOLIDAYS; i++){
    if(holidays[i].month == month && holidays[i].day == day){
      strcpy(dest,"1");
      return 0;
    }
  }
  strcpy(dest,"0");
  return 0;
}

int
is_last_day_in_month(int year,int month,int day){
    int * days;
    if(LEAP(year))
      days = days_in_a_month_l;
    else
      days = days_in_a_month;
    if(day ==  days[month-1]) return 1;
    return 0;
}

int gen_season(char * dest,int month,int day)
{
    int i;
    for(i =0;i<NUM_SEASONS;i++){
	season *seas;
	seas = &seasons[i];
        
	if(month>=seas->start_month && month<=seas->end_month && 
	   day >= seas->start_day && day <= seas->end_day){
	    strcpy(dest, seas->name);
            return 0;
        }
    }
    strcpy(dest,"");

    return 0;
}
