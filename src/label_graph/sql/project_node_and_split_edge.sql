CREATE OR REPLACE FUNCTION pgr_projectNodeAndSplitEdge(
                edge_table_rows text,
                points_table_rows text,
                projectnodeandsplitedge_table text
        )
        RETURNS character varying AS
$BODY$

DECLARE
        naming record;
        x_pt double precision;
        y_pt double precision;
        srid integer;
        schema_name text;
        table_name text;
        table_schema_name text;
        query text;
        ecnt integer;
        sql1 text;
        rec1 record;
        sql2 text;
        rec2 record;
        rec_break record;
        rec_single record;
        graph_id integer;
        gids int [];   
BEGIN
        raise notice 'Processing:';
        raise notice 'pgr_projectNodeAndSplitEdge(''%'',''%'')', edge_table_rows, points_table_rows;
        raise notice 'Performing initial checks, please hold on ...';

        Raise Notice 'Starting - Checking edge_table_rows condition';
        BEGIN
                raise debug 'Checking edge_table_rows condition';
                EXECUTE 'with tbl as ('|| edge_table_rows ||') select count(*) from tbl' into ecnt;
                raise debug '-->Rows where condition: OK';
                raise debug '    --> OK';
                EXCEPTION WHEN OTHERS THEN
                        raise notice 'Got %', SQLERRM;
                        Raise notice 'ERROR: edge_table_rows condition is not correct. Please execute the following query to test your condition';
                        Raise notice '%', query;
                        return 'FAIL';
        END;
        Raise Notice 'Ending - Checking edge_table_rows condition';

        Raise Notice 'Starting - Checking points_table_rows condition';
        BEGIN
                raise debug 'Checking edge_table_rows condition';
                EXECUTE 'with tbl as ('|| points_table_rows ||') select count(*) from tbl' into ecnt;
                raise debug '-->Rows where condition: OK';
                raise debug '    --> OK';
                EXCEPTION WHEN OTHERS THEN
                        raise notice 'Got %', SQLERRM;
                        Raise notice 'ERROR: points_table_rows condition is not correct. Please execute the following query to test your condition';
                        Raise notice '%', query;
                        return 'FAIL';
        END;
        Raise Notice 'Ending - Checking points_table_rows condition';

        Raise Notice 'Starting - Checking future table existance ...';
        BEGIN
                raise debug 'Checking % table existance', projectnodeandsplitedge_table;
                execute 'select * from pgr_getTableName('|| quote_literal(projectnodeandsplitedge_table) ||')' into naming;
                schema_name = naming.sname;
                table_name = naming.tname;
                table_schema_name = schema_name||'.'||table_name;
                IF schema_name is null then
                        raise notice 'no schema';
                        return 'FAIL';
                else 
                        if table_name is not null then
                                raise notice '% table already in % schema', table_name, schema_name;
                                return 'FAIL';
                        end if;
                end if;
        END;
        Raise Notice 'Ending - Checking future table existance';

        BEGIN
                --------- Make new table to hold edge_table rows for further alteration. Thus original edge_table will remain unaltered ----------
                EXECUTE 'CREATE TABLE ' || pgr_quote_ident(projectnodeandsplitedge_table) || ' AS '|| edge_table_rows ||'';
                sql1 := ''|| points_table_rows ||'';
                For rec1 in execute sql1 
                        Loop
                                x_pt := st_x(rec1.geom);
                                y_pt := st_y(rec1.geom);
                                srid := st_srid(rec1.geom);
                                Execute 'with line as (select geom from '||pgr_quote_ident(projectnodeandsplitedge_table)||' order by st_distance(geom, ST_GeomFromText(''POINT('||x_pt||' '||y_pt||')'', '||srid||')) limit 1),
                                        frac as (select ST_LineLocatePoint(st_linemerge(line.geom),ST_GeomFromText(''POINT('||x_pt||' '||y_pt||')'', '||srid||')) as fraction from line)
                                        select ST_LineSubstring (line.geom,0,frac.fraction) as first_half,
                                        ST_LineSubstring (line.geom,frac.fraction,1) as second_half
                                        from frac, line' into rec_break;
                                insert into xx values (rec_break.first_half);
                                raise notice '%', rec_break.first_half;
                                --raise notice '%', st_x(rec1.geom);
                                --raise notice 'ji'; 
                        End Loop;
        END;
        

        RETURN 'OK';
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;
