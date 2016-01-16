/*PGR-GNU*****************************************************************

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

********************************************************************PGR-GNU*/
/*
This test is for the equivalence pgr_turnRestricted when:
- no restrictions are given 
- this one autodetects if has_rcost so no contradiction tests

With pgr_dijkstra one To One signature
for: directed versions

*/

\set ECHO none
\set QUIET 1
-- Turn off echo and keep things quiet.

-- Format the output for nice TAP.
\pset format unaligned
\pset tuples_only true
\pset pager
\set VERBOSITY terse

-- Revert all changes on failure.
\set ON_ERROR_ROLLBACK true
\set ON_ERROR_STOP true
\set QUIET 1

BEGIN;
    SELECT plan(612);

    UPDATE edge_table SET cost = cost + 0.001 * id * id, reverse_cost = reverse_cost + 0.001 * id * id;


    CREATE OR REPLACE FUNCTION foo(cant INTEGER default 18, flag boolean default true )
    RETURNS SETOF TEXT AS
    $BODY$
    DECLARE
    dijkstra_sql TEXT;
    turnRestricted_sql TEXT;
    inner_sql1 TEXT;
    inner_sql2 TEXT;
    k integer;
    directed TEXT;
    msg TEXT;
    BEGIN
        directed = 'Undirected';
        IF flag THEN directed = 'Directed'; END IF;
        k := 1;
        inner_sql1 = quote_literal('SELECT id, source, target, cost, reverse_cost from edge_table ORDER BY id');
        inner_sql2 = quote_literal('SELECT id, source, target, cost from edge_table ORDER BY id');
        FOR i IN 1.. cant LOOP
            FOR j IN 1..cant LOOP

                -- this special case is tested on the other test
                CONTINUE WHEN i = j;

                -- test when there is reverse cost
                -- VS dijkstra autodetected has reverse cost
                dijkstra_sql := 'SELECT seq-1, node::integer, edge::integer, cost::text
                    FROM pgr_dijkstra( ' || inner_sql1 || ', ' || i || ', ' || j || ', ' || flag || ')';
                turnRestricted_sql := 'SELECT seq, id1, id2, cost::text from pgr_turnRestricted( ' || inner_sql1 || ', ' || i || ', ' || j || ', ' || flag || ')';
                msg := k || '-1 ' || directed || ', with reverse_cost, marked as being used: from '  || i || ' to ' || j;
                BEGIN
                    EXECUTE turnRestricted_sql;

                    -- This should happen allways even when its being corrected
                    RETURN query SELECT set_eq(turnRestricted_sql, dijkstra_sql, msg);
                    EXCEPTION WHEN OTHERS THEN
                        RETURN query SELECT is_empty(dijkstra_sql, msg);
                END;

                -- THIS NEVER HAPPENS HERE
                -- test when there is reverse cost and its marked NOT being used
                -- VS dijkstra autodetected DOES NOT have reverse cost
                /*
                dijkstra_sql := 'SELECT seq-1, node::integer, edge::integer, cost::text
                    FROM pgr_dijkstra( ' || inner_sql2 || ', ' || i || ', ' || j || ', ' || flag || ')';
                turnRestricted_sql := 'SELECT seq, id1, id2, cost::text from pgr_turnRestricted( ' || inner_sql1 || ', ' || i || ', ' || j || ', ' || flag || ', FALSE)';
                msg := k || '-2 ' || directed || ', with reverse_cost, marked as NOT being used: from '  || i || ' to ' || j;
                BEGIN
                    EXECUTE turnRestricted_sql;

                    -- This should happen allways even when its being corrected
                    RETURN query SELECT set_eq(turnRestricted_sql, dijkstra_sql, msg);
                    EXCEPTION WHEN OTHERS THEN
                        msg := k || '-2 ' || directed || ', with reverse_cost, marked as NOT being used: from '  || i || ' to ' || j || '... Exception found';
                        RETURN query SELECT is_empty(dijkstra_sql, msg);
                END;
                */

                -- test when there is NO reverse cost and its marked NOT being used
                -- VS dijkstra autodetected DOES NOT have reverse cost
                dijkstra_sql := 'SELECT seq-1, node::integer, edge::integer, cost::text
                    FROM pgr_dijkstra( ' || inner_sql2 || ', ' || i || ', ' || j || ', ' || flag || ')';
                turnRestricted_sql := 'SELECT seq, id1, id2, cost::text from pgr_turnRestricted( ' || inner_sql2 || ', ' || i || ', ' || j || ', ' || flag || ')';
                msg := k || '-3 ' || directed || ', NO reverse_cost, marked as NOT being used: from '  || i || ' to ' || j;
                BEGIN
                    EXECUTE turnRestricted_sql;

                    -- This should happen allways even when its being corrected
                    RETURN query SELECT set_eq(turnRestricted_sql, dijkstra_sql, msg);

                    EXCEPTION WHEN OTHERS THEN
                        RETURN query SELECT is_empty(dijkstra_sql, msg);
                END;

                -- THIS NEVER HAPPENS HERE
                -- test when there is NO reverse cost and its marked  AS being used
                -- Uncomparable with dijstra because dijstra uses what is given as input
                /*
                turnRestricted_sql := 'SELECT * from pgr_turnRestricted( ' || inner_sql2 || ', ' || i || ', ' || j || ', ' || flag || ', TRUE)';
                msg := k || '-4 ' || directed || ', NO reverse_cost, marked as NOT being used: from '  || i || ' to ' || j;
                RETURN query SELECT throws_ok(turnRestricted_sql,'XX000','Error, reverse_cost is used, but query did''t return ''reverse_cost'' column', msg);
                k := k + 1;
                */

            END LOOP;
        END LOOP;
    END
$BODY$
language plpgsql;

SELECT * from foo(18, true);
--SELECT * from foo(18, false);


-- Finish the tests and clean up.
SELECT * FROM finish();
ROLLBACK;
