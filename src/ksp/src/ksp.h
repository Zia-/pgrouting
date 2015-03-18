/*
 * KShortest path algorithm for PostgreSQL
 *
 * Copyright (c) 2012 Dave Potts
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
 
#ifndef _KSP_H
#define _KSP_H

#define TUPLIMIT 1000
#define PATH_ALLOC_TOTAL 5

#include "postgres.h"
#include "./../../common/src/postgres_types.h"
#include "./../../common/src/postgres_connection.h"

#undef DEBUG
//#define DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

int compute_kshortest_path(char* sql, int64_t start_vertex, 
                                 int64_t end_vertex, int no_paths, 
                                 bool has_reverse_cost, 
                                 pgr_path_element3_t **path, int *ksp_path_count) ;

#ifdef __cplusplus

}
#endif

#endif // _KSP_H
