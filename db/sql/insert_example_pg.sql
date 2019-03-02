INSERT INTO public.user VALUES
('user1','123','admin'),
('user2','123','actor'),
('user3','123','viewer');
update public.user set session_stop_time=(localtimestamp + cast(to_char(3600, '00000000000 "S"') as interval)), session_id='232333' where name='user1' and pswd='123';

INSERT INTO bb.rack VALUES
(1,49162,'127.0.0.1'),
(2,49163,'127.0.0.1'),
(3,49191,'127.0.0.1'),
(4,49173,'127.0.0.1'),
(5,49184,'127.0.0.1'),
(6,49173,'127.0.0.1'),
(7,49173,'127.0.0.1'),
(8,49173,'127.0.0.1'),
(9,49173,'127.0.0.1'),
(10,49173,'127.0.0.1'),
(11,49173,'127.0.0.1'),
(12,49173,'127.0.0.1');

INSERT INTO bb.group VALUES
(1,'ЗАО Овощи'),
(2,'теплица 1'),
(3,'теплица 2');

INSERT INTO bb.group_group VALUES
(1,2),
(1,3);

INSERT INTO bb.group_rack VALUES
(2,1),
(2,2),
(2,3),
(2,4),
(2,5),
(2,6),
(3,7),
(3,8),
(3,9),
(3,10),
(3,11),
(3,12);

DROP FUNCTION bb.in_fly(integer, integer, integer, integer, integer);
CREATE OR REPLACE FUNCTION bb.in_fly(
    rack_num integer,
    hive_num integer,
    max_val integer,
    hr integer,
    lr integer
    )
  RETURNS integer AS
$BODY$declare
 mr integer;
 val integer;
 tm bigint;
begin
    tm=1;
    delete from bb.fly;
    FOR r IN 1..rack_num LOOP
        FOR h IN 1..hive_num LOOP
            val = max_val;
            
            WHILE val > 0 LOOP
                --select extract(epoch from localtimestamp)::bigint into tm;
                insert into bb.fly(rack_id, hive_id, value, period, mark) values (r, h, val, cast(to_char(300, '00000000000 "S"') as interval), tm);
               -- perform bb.save_fly(r,h,val,100,300);
               -- perform pg_sleep(2);
               SELECT floor(random() * (hr-lr+1) + lr)::int into mr;
               val = val - mr;
               tm = tm+1;
            END LOOP;
        END LOOP;
    END LOOP;
    return 1;
end;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

  
select bb.in_fly(12,3,500,10,3);
select * from bb.fly where rack_id=1 and hive_id=1 order by value desc;

DROP FUNCTION bb.in_installed(integer, integer, integer, integer);
CREATE OR REPLACE FUNCTION bb.in_installed(
    rack_num integer,
    hive_num integer,
    hr integer,
    lr integer
    )
  RETURNS integer AS
$BODY$declare
 mr integer;
 val timestamp without time zone;
begin
    delete from bb.installed;
    FOR r IN 1..rack_num LOOP
        FOR h IN 1..hive_num LOOP
            select localtimestamp into val;
            FOR i IN 1..10 LOOP
                insert into bb.installed(rack_id, hive_id, value) values (r, h, val);
                SELECT floor(random() * (hr-lr+1) + lr)::int into mr;
                val = val - cast( to_char(mr,'00000000000 "S"') as interval);
            END LOOP;
        END LOOP;
    END LOOP;
    return 1;
end;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

select bb.in_installed(12,3,7776000,518400);
select * from bb.installed where rack_id=1 and hive_id=1 order by value desc;


DROP FUNCTION bb.in_ht(integer, integer, integer, integer);
CREATE OR REPLACE FUNCTION bb.in_ht(
    rack_num integer,
    hive_num integer,
    num integer,
    hrt real,
    lrt real,
    hrh real,
    lrh real
    )
  RETURNS integer AS
$BODY$declare
    tm timestamp without time zone;
    valt real;
    valh real;
    mr integer;
begin
    delete from bb.temperature;
    delete from bb.humidity;
    mr = 300;
    FOR r IN 1..rack_num LOOP
        FOR h IN 1..hive_num LOOP
            select localtimestamp into tm;
            FOR i IN 1..num LOOP
                SELECT random() * (hrt-lrt+1) + lrt into valt;
                SELECT random() * (hrh-lrh+1) + lrh into valh;
                insert into bb.temperature(id, value, mark) values (r*100 + h, valt, tm);
                insert into bb.humidity(id, value, mark) values (r*100 + h, valh, tm);
                tm = tm - cast( to_char(mr,'00000000000 "S"') as interval);
            END LOOP;
        END LOOP;
    END LOOP;
    return 1;
end;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

select bb.in_ht(12,3,1000,45.0,10.0,95.0,60.0);
select * from bb.temperature where rack_id=1 and hive_id=1 order by mark desc;
