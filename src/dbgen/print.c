/*
* $Id: print.c,v 1.3 2005/10/28 02:56:22 jms Exp $
*
* Revision History
* ===================
* $Log: print.c,v $
* Revision 1.3  2005/10/28 02:56:22  jms
* add platform-specific printf formats to allow for DSS_HUGE data type
*
* Revision 1.2  2005/01/03 20:08:59  jms
* change line terminations
*
* Revision 1.1.1.1  2004/11/24 23:31:47  jms
* re-establish external server
*
* Revision 1.4  2004/02/18 16:26:49  jms
* 32/64 bit changes for overflow handling needed additional changes when ported back to windows
*
* Revision 1.3  2004/02/18 14:05:53  jms
* porting changes for LINUX and 64 bit RNG
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
/* generate flat files for data load */
#include <stdio.h>
#ifndef VMS
#include <sys/types.h>
#endif
#if defined(SUN)
#include <unistd.h>
#endif
#include <math.h>

#include "dss.h"
#include "dsstypes.h"
#include <string.h>

/*
 * Function Prototypes
 */
FILE *print_prep PROTO((int table, int update));
int pr_drange PROTO((int tbl, DSS_HUGE min, DSS_HUGE cnt, long num));

FILE *
print_prep(int table, int update)
{
	char upath[128];
	FILE *res;

	if (updates)
		{
		if (update > 0) /* updates */
			if ( insert_segments )
				{
				int this_segment;
				if(strcmp(tdefs[table].name,"orders.tbl"))
					this_segment=++insert_orders_segment;
				else 
					this_segment=++insert_lineitem_segment;
				sprintf(upath, "%s%c%s.u%d.%d", 
					env_config(PATH_TAG, PATH_DFLT),
					PATH_SEP, tdefs[table].name, update%10000,this_segment);
				}
			else
				{
				sprintf(upath, "%s%c%s.u%d",
				env_config(PATH_TAG, PATH_DFLT),
				PATH_SEP, tdefs[table].name, update);
				}
		else /* deletes */
			if ( delete_segments )
				{
				++delete_segment;
				sprintf(upath, "%s%cdelete.u%d.%d",
					env_config(PATH_TAG, PATH_DFLT), PATH_SEP, -update%10000,
					delete_segment);
				}
			else
				{
				sprintf(upath, "%s%cdelete.%d",
				env_config(PATH_TAG, PATH_DFLT), PATH_SEP, -update);
				}
		return(fopen(upath, "w"));
        }
    res = tbl_open(table, "w");
    OPEN_CHECK(res, tdefs[table].name);
    return(res);
}

int
dbg_print(int format, FILE *target, void *data, int len, int sep)
{
	int dollars,
		cents;

	switch(format)
	{
	case DT_STR:
		fprintf(target, "%s", (char *)data);
		break;
#ifdef MVS
	case DT_VSTR:
		/* note: only used in MVS, assumes columnar output */
		fprintf(target, "%c%c%-*s", 
			(len >> 8) & 0xFF, len & 0xFF, len, (char *)data);
		break;
#endif /* MVS */
	case DT_INT:
		fprintf(target, "%ld", (long)data);
		break;
	case DT_HUGE:
		fprintf(target, HUGE_FORMAT, *(DSS_HUGE *)data);
		break;
	case DT_KEY:
		fprintf(target, "%ld", (long)data);
		break;
	case DT_MONEY:
		cents = (int)*(DSS_HUGE *)data;
		if (cents < 0)
			{
			fprintf(target, "-");
			cents = -cents;
			}
		dollars = cents / 100;
		cents %= 100;
		fprintf(target, "%d.%02d", dollars, cents);
		break;
	case DT_CHR:
		fprintf(target, "%c", *(char *)data);
		break;
	}

#ifdef EOL_HANDLING
	if (sep)
#endif /* EOL_HANDLING */
	fprintf(target, "%c", (char) SEPARATOR);
	
	return(0);
}

int
pr_cust(customer_t *c, int mode)
{
static FILE *fp = NULL;
        
   if (fp == NULL)
        fp = print_prep(CUST, 0);

   PR_STRT(fp);
   PR_HUGE(fp, &c->custkey);
   if (scale <= 3000)
   PR_VSTR(fp, c->name, C_NAME_LEN);
   else
   PR_VSTR(fp, c->name, C_NAME_LEN + 3);
   PR_VSTR(fp, c->address, 
       (columnar)?(long)(ceil(C_ADDR_LEN * V_STR_HGH)):c->alen);
   PR_STR(fp, c->city,CITY_FIX);
   PR_STR(fp, c->nation_name, C_NATION_NAME_LEN);
   PR_STR(fp, c->region_name, C_REGION_NAME_LEN);
   PR_STR(fp, c->phone, PHONE_LEN);
   PR_STR(fp, c->mktsegment,MAXAGG_LEN);
   PR_END(fp);

   return(0);
}

/*
 * print the dates
 */
int
pr_date(date_t *d, int mode)
{
    static FILE *d_fp = NULL;
    static int last_mode = 0;
        
    if (d_fp == NULL || mode != last_mode)
        {
        if (d_fp) 
            fclose(d_fp);
        d_fp = print_prep(DATE, mode);
        last_mode = mode;
        }
    PR_STRT(d_fp);
    PR_INT(d_fp, d->datekey);
    PR_STR(d_fp, d->date,D_DATE_LEN);
    PR_STR(d_fp, d->dayofweek,D_DAYWEEK_LEN);
    PR_STR(d_fp, d->month,D_MONTH_LEN);
    PR_INT(d_fp, d->year);
    PR_INT(d_fp, d->yearmonthnum);
    PR_STR(d_fp, d->yearmonth,D_YEARMONTH_LEN);
    PR_INT(d_fp, d->daynuminweek);
    PR_INT(d_fp, d->daynuminmonth);
    PR_INT(d_fp, d->daynuminyear);
    PR_INT(d_fp, d->monthnuminyear);
    PR_INT(d_fp, d->weeknuminyear);
    PR_VSTR(d_fp, 
	    d->sellingseason,(columnar)?(long)D_SEASON_LEN:d->slen);
    PR_STR(d_fp,d->lastdayinweekfl,2);
    PR_STR(d_fp,d->lastdayinmonthfl,2);
    PR_STR(d_fp,d->holidayfl,2);
    PR_STR(d_fp,d->weekdayfl,2);
    PR_END(d_fp);

    return(0);
}

/*
 * print the lineorders
 */
int
pr_order(order_t *o, int mode)
{
    static FILE *fp_l = NULL;
    static int last_mode = 0;
    long      i;
        
    if (fp_l == NULL || mode != last_mode)
        {
        if (fp_l) 
            fclose(fp_l);
        fp_l = print_prep(LINE, mode);
        last_mode = mode;
        }
    for (i = 0; i < o->lines; i++)
        {
        PR_STRT(fp_l);
        PR_HUGE(fp_l, &o->lineorders[i].okey);
        PR_INT(fp_l, o->lineorders[i].linenumber);
	PR_INT(fp_l, o->lineorders[i].custkey);
	PR_INT(fp_l, o->lineorders[i].partkey);
        PR_INT(fp_l, o->lineorders[i].suppkey);
        PR_STR(fp_l, o->lineorders[i].orderdate, DATE_LEN);
	PR_STR(fp_l, o->lineorders[i].opriority, O_OPRIO_LEN);
	PR_INT(fp_l, o->lineorders[i].ship_priority);
        PR_INT(fp_l, o->lineorders[i].quantity);
        PR_INT(fp_l, o->lineorders[i].extended_price);
        PR_INT(fp_l, o->lineorders[i].order_totalprice);
        PR_INT(fp_l, o->lineorders[i].discount);
        PR_INT(fp_l, o->lineorders[i].revenue);
	PR_INT(fp_l, o->lineorders[i].supp_cost);
	PR_INT(fp_l, o->lineorders[i].tax);
	PR_STR(fp_l, o->lineorders[i].commit_date, DATE_LEN);
	PR_STR(fp_l, o->lineorders[i].shipmode, O_SHIP_MODE_LEN);
        PR_END(fp_l);
        }


   return(0);
}

/*
 * print the given part
 */
int
pr_part(part_t *part, int mode)
{
static FILE *p_fp = NULL;

    if (p_fp == NULL)
        p_fp = print_prep(PART, 0);

   PR_STRT(p_fp);
   PR_HUGE(p_fp, &part->partkey);
   PR_VSTR(p_fp, part->name,
           (columnar)?(long)P_NAME_LEN:part->nlen);
   PR_STR(p_fp, part->mfgr, P_MFG_LEN);
   PR_STR(p_fp, part->category, P_CAT_LEN);
   PR_STR(p_fp, part->brand, P_BRND_LEN);

   /*need to handle color*/
   PR_VSTR(p_fp, part->color,(columnar)?(long)P_COLOR_LEN:part->clen);
   PR_VSTR(p_fp, part->type,
	   (columnar)?(long)P_TYPE_LEN:part->tlen);
   PR_INT(p_fp, part->size);
   PR_STR(p_fp, part->container, P_CNTR_LEN);
   PR_END(p_fp);

   return(0);
}

/*
 * print the given suppliers
 */

int
pr_supp(supplier_t *supp, int mode)
{
static FILE *fp = NULL;
        
   if (fp == NULL)
        fp = print_prep(SUPP, mode);

   PR_STRT(fp);
   PR_HUGE(fp, &supp->suppkey);
   PR_STR(fp, supp->name, S_NAME_LEN);
    
   PR_VSTR(fp, supp->address,
	   (columnar)?(long)(ceil(S_ADDR_LEN * V_STR_HGH)):supp->alen);
   PR_STR(fp, supp->city, CITY_FIX);
   PR_STR(fp, supp->nation_name, C_NATION_NAME_LEN);
   PR_STR(fp, supp->region_name, C_REGION_NAME_LEN);
   PR_STR(fp, supp->phone, PHONE_LEN);
   PR_END(fp);

   return(0);
}

/* 
 * NOTE: this routine does NOT use the BCD2_* routines. As a result,
 * it WILL fail if the keys being deleted exceed 32 bits. Since this
 * would require ~660 update iterations, this seems an acceptable
 * oversight
 */
int
pr_drange(int tbl, DSS_HUGE min, DSS_HUGE cnt, long num)
{
    static int  last_num = 0;
    static FILE *dfp = NULL;
    DSS_HUGE child = -1;
    DSS_HUGE start, last, new;

	static DSS_HUGE rows_per_segment=0;
	static DSS_HUGE rows_this_segment=0;

    if (last_num != num)
        {
        if (dfp)
            fclose(dfp);
        dfp = print_prep(tbl, -num);
        if (dfp == NULL)
            return(-1);
        last_num = num;
		rows_this_segment=0;
        }

    start = MK_SPARSE(min, num/ (10000 / UPD_PCT));
    last = start - 1;
    for (child=min; cnt > 0; child++, cnt--)
	{
		new = MK_SPARSE(child, num/ (10000 / UPD_PCT));
		if (delete_segments)
		{

			if(rows_per_segment==0) 
				rows_per_segment = (cnt / delete_segments) + 1;
			if((++rows_this_segment) > rows_per_segment)
			{
				fclose(dfp);
				dfp = print_prep(tbl, -num);
				if (dfp == NULL) return(-1);
				last_num = num;
				rows_this_segment=1;
			}
		}
		PR_STRT(dfp);
		PR_HUGE_LAST(dfp, &new);
		PR_END(dfp);
		start = new;
		last = new;
	}
    
    return(0);
}



