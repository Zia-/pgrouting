/*
 * kShortest path algorithm for PostgreSQL
 *
 * Copyright (c) 2011 Dave Potts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"
#include "fmgr.h"
#if PGSQL_VERSION > 92
#include "access/htup_details.h"
#endif

#include "./ksp.h" 
#include "./../../common/src/postgres_types.h" 
#include "./../../common/src/postgres_connection.h" 
#include "KSPDriver.h"

#ifndef _MSC_VER
Datum kshortest_path(PG_FUNCTION_ARGS);
#else // _MSC_VER
PGDLLEXPORT Datum kshortest_path(PG_FUNCTION_ARGS);
#endif // _MSC_VER



#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


PG_FUNCTION_INFO_V1(kshortest_path);
#ifndef _MSC_VER
Datum
#else // _MSC_VER
PGDLLEXPORT Datum
#endif // _MSC_VER

kshortest_path(PG_FUNCTION_ARGS)
{
  FuncCallContext     *funcctx;
  int                  call_cntr;
  int                  max_calls;
  TupleDesc            tuple_desc;
  pgr_path_element3_t      *path;
//  void * toDel;
  //int                  ret;

  /* stuff done only on the first call of the function */
  if (SRF_IS_FIRSTCALL())
    {
      MemoryContext   oldcontext;
      int path_count = 0;
      //int ret;

      /* create a function context for cross-call persistence */
      funcctx = SRF_FIRSTCALL_INIT();

      /* switch to memory context appropriate for multiple function calls */
      oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);


      int ret = compute_kshortest_path(pgr_text2char(PG_GETARG_TEXT_P(0)), /* SQL string */
                                  PG_GETARG_INT64(1),             /* source id */
                                  PG_GETARG_INT64(2),             /* target_id */
                                  PG_GETARG_INT32(3),             /* number of paths */
                                  PG_GETARG_BOOL(4), 		  /* has reverse_cost */
				  &path,
				  &path_count);
//      toDel=path;
//#ifdef DEBUG
      if (ret >= 0) 
        {
          int i;
        
          for (i = 0; i < path_count; i++) 
            {
              DBG("Step %i route_id  %d ", i, path[i].route_id);
              DBG("        vertex_id  %ld ", path[i].vertex_id);
              DBG("        edge_id    %ld ", path[i].edge_id);
              DBG("        cost       %f ", path[i].cost);
            }
        }
//#endif
     DBG("Total number of tuples to be returned %i ", path_count);
	
      /* total number of tuples to be returned */
      funcctx->max_calls = path_count;
      funcctx->user_fctx = path;

      funcctx->tuple_desc = 
        BlessTupleDesc(RelationNameGetTupleDesc("pgr_costResult3Big"));

      MemoryContextSwitchTo(oldcontext);
    }


  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();

  call_cntr = funcctx->call_cntr;
  max_calls = funcctx->max_calls;
  tuple_desc = funcctx->tuple_desc;
  path = (pgr_path_element3_t*) funcctx->user_fctx;

  if (call_cntr < max_calls)    /* do when there is more left to send */
    {
      HeapTuple    tuple;
      Datum        result;
     /* //Datum values[4];
      //bool nulls[4]; */

      Datum *values;
      bool* nulls;

      values = (Datum *)palloc(5 * sizeof(Datum));
      nulls =(bool *) palloc(5 * sizeof(bool));


      values[0] = Int32GetDatum(call_cntr);
      nulls[0] = false;
      values[1] = Int32GetDatum(path[call_cntr].route_id);
      nulls[1] = false;
      values[2] = Int64GetDatum(path[call_cntr].vertex_id);
      nulls[2] = false;
      values[3] = Int64GetDatum(path[call_cntr].edge_id);
      nulls[3] = false;
      values[4] = Float8GetDatum(path[call_cntr].cost);
      nulls[4] = false;

      tuple = heap_form_tuple(tuple_desc, values, nulls);

      /* make the tuple into a datum */
      result = HeapTupleGetDatum(tuple);

      /* clean up (this is not really necessary) */
      pfree(values);
      pfree(nulls);

      SRF_RETURN_NEXT(funcctx, result);
    }
  else    /* do when there is no more left */
    {
	free(path);
      SRF_RETURN_DONE(funcctx);
    }
}



int compute_kshortest_path(char* sql, int64_t start_vertex, 
			 int64_t end_vertex, int no_paths, 
			 bool has_reverse_cost, 
			 pgr_path_element3_t **ksp_path, int *path_count) 
{

  int SPIcode;
  int ret = -1;
  long total_tuples = 0;
  pgr_edge_t *edges = NULL;
  char *err_msg=(char *)"";
#if 0
  SPIcode = pgr_retrieve_data_from_sql( sql, edges, &total_tuples, has_reverse_cost);
  if (SPIcode == -1) return -1;

#else
  
  void *SPIplan;
  Portal SPIportal;
  bool moredata = TRUE;
  int ntuples;
#ifndef _MSC_VER
  pgr_edge_t edge_columns = {.id= -1, .source= -1, .target= -1, 
                                 .cost= -1, .reverse_cost= -1};
#else // _MSC_VER
  pgr_edge_t edge_columns = {-1, -1, -1, -1, -1};
#endif // _MSC_VER
#if 0
  long v_max_id=0;
  long v_min_id=LONG_MAX;
  long s_count = 0;
  long t_count = 0;
#endif


  DBG("Starting kshortest_path %s\n",sql);
        
  SPIcode = SPI_connect();
  if (SPIcode  != SPI_OK_CONNECT)
    {
      elog(ERROR, "kshortest_path: couldn't open a connection to SPI");
      return -1;
    }

  SPIplan = SPI_prepare(sql, 0, NULL);
  if (SPIplan  == NULL)
    {
      elog(ERROR, "kshortest_path: couldn't create query plan via SPI");
      return -1;
    }

  if ((SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true)) == NULL) 
    {
      elog(ERROR, "shortest_path: SPI_cursor_open('%s') returns NULL", sql);
      return -1;
    }

  while (moredata == TRUE)
    {
      SPI_cursor_fetch(SPIportal, TRUE, TUPLIMIT);

      if (edge_columns.id == -1) 
        {
          if (pgr_fetch_edge_columns(SPI_tuptable, &edge_columns, 
                                 has_reverse_cost) == -1)
	    return pgr_finish(SPIcode, ret);
        }
      ntuples = SPI_processed;
      total_tuples += ntuples;
      if (!edges)
        edges = (pgr_edge_t *)palloc(total_tuples * sizeof(pgr_edge_t));
      else
        edges = (pgr_edge_t *)repalloc(edges, total_tuples * sizeof(pgr_edge_t));

      if (edges == NULL) 
        {
          elog(ERROR, "Out of memory");
	    return pgr_finish(SPIcode, ret);	  
        }

      if (ntuples > 0) 
        {
          int t;
          SPITupleTable *tuptable = SPI_tuptable;
          TupleDesc tupdesc = SPI_tuptable->tupdesc;
                
          for (t = 0; t < ntuples; t++) 
            {
              HeapTuple tuple = tuptable->vals[t];
              pgr_fetch_edge(&tuple, &tupdesc, &edge_columns, 
                         &edges[total_tuples - ntuples + t]);
            }
          SPI_freetuptable(tuptable);
        } 
      else 
        {
          moredata = FALSE;
        }
    }

 
      
  DBG("Total %ld tuples in query", total_tuples);
 #endif 

  DBG("Calling doKpaths\n");
        
  ret = doKpaths(edges, total_tuples,
			start_vertex, end_vertex,
                       no_paths, has_reverse_cost,
                       ksp_path, path_count, &err_msg);
  DBG("total tuples found %i\n",*path_count);
  DBG("Exist Status = %i\n", ret);
  DBG("Returned message = %s\n", err_msg);

  
  
  if (ret < 0)
    {
      //elog(ERROR, "Error computing path: %s", err_msg);
      ereport(ERROR, (errcode(ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED), 
      errmsg("Error computing path: %s", err_msg)));
    } 
    
  pfree(edges);
  return pgr_finish(SPIcode, ret);
}
