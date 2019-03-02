DROP TABLE public.user;
CREATE TABLE public.user
(
  name varchar(16) NOT NULL,
  pswd varchar(16) NOT NULL,
  kind varchar(16) NOT NULL,
  session_id varchar(16),
  session_stop_time timestamp without time zone,
  CONSTRAINT user_pkey PRIMARY KEY (name)
)
WITH (
  OIDS=FALSE
);

CREATE TABLE public.user_action
(
    user_name varchar(16) NOT NULL,
    action varchar(32) NOT NULL,
    mark timestamp without time zone NOT NULL,
    CONSTRAINT useraction_pkey PRIMARY KEY (mark)
)
WITH (
  OIDS=FALSE
);

-- DROP SCHEMA if exists "bb" CASCADE;
CREATE SCHEMA "bb";

-- DROP TABLE bb.rack;
CREATE TABLE bb.rack
(
  id integer NOT NULL,
  port integer NOT NULL,
  ip_addr varchar(16) NOT NULL,
  CONSTRAINT rack_pkey PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);

-- DROP TABLE bb.group;
CREATE TABLE bb.group
(
  id integer NOT NULL,
  name varchar(32) NOT NULL,
  CONSTRAINT group_pkey PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);

-- DROP TABLE bb.group_group;
CREATE TABLE bb.group_group
(
    parent_id integer NOT NULL,
    child_id integer NOT NULL,
    CONSTRAINT group_group_pkey PRIMARY KEY (parent_id, child_id)
)
WITH (
  OIDS=FALSE
);

-- DROP TABLE bb.group_rack;
CREATE TABLE bb.group_rack
(
    group_id integer NOT NULL,
    rack_id integer NOT NULL,
    CONSTRAINT group_rack_pkey PRIMARY KEY (group_id, rack_id)
)
WITH (
  OIDS=FALSE
);

DROP TABLE bb.fly;
CREATE TABLE bb.fly
(
  rack_id integer NOT NULL,
  hive_id integer NOT NULL,
  value integer NOT NULL,
  period interval NOT NULL,
  mark bigint NOT NULL,
  CONSTRAINT fly_pkey PRIMARY KEY (rack_id, hive_id, mark)
)
WITH (
  OIDS=FALSE
);

-- DROP TABLE bb.installed;
CREATE TABLE bb.installed
(
  rack_id integer NOT NULL,
  hive_id integer NOT NULL,
  value timestamp without time zone NOT NULL,
  CONSTRAINT installed_pkey PRIMARY KEY (rack_id, hive_id, value)
)
WITH (
  OIDS=FALSE
);

DROP TABLE bb.temperature;
CREATE TABLE bb.temperature
(
  id integer NOT NULL,
  value real NOT NULL,
  mark timestamp without time zone NOT NULL,
  CONSTRAINT temperature_pkey PRIMARY KEY (id, mark)
)
WITH (
  OIDS=FALSE
);

DROP TABLE bb.humidity;
CREATE TABLE bb.humidity
(
  id integer NOT NULL,
  value real NOT NULL,
  mark timestamp without time zone NOT NULL,
  CONSTRAINT humidity_pkey PRIMARY KEY (id, mark)
)
WITH (
  OIDS=FALSE
);


DROP FUNCTION bb.save_fly(integer, integer, integer, integer, integer);

CREATE OR REPLACE FUNCTION bb.save_fly(
    in_rack_id integer,
    in_hive_id integer,
    val integer,
    max_rows integer,
    in_period integer
    )
  RETURNS integer AS
$BODY$declare
 n bigint;
 in_mark bigint;
begin
  select extract(epoch from localtimestamp)::bigint into in_mark;
  select count(*) from bb.fly where rack_id=in_rack_id and hive_id=in_hive_id into n;
  if not FOUND then
    raise exception 'count failed where rack_id=% and hive_id=% ', in_rack_id, in_hive_id;
  end if;
  if n < max_rows then
    insert into bb.fly(rack_id, hive_id, value, period, mark) values (in_rack_id, in_hive_id, val, cast(to_char(in_period, '00000000000 "S"') as interval), in_mark);
    if not FOUND then
      raise exception 'insert failed where rack_id=% and hive_id=% ', in_rack_id, in_hive_id;
    end if;
    return 1;
  else
   update bb.fly set period=cast(to_char(in_period, '00000000000 "S"') as interval), mark=in_mark, value=val where rack_id=in_rack_id and hive_id=in_hive_id and mark=(select min(mark) from bb.fly where rack_id=in_rack_id and hive_id=in_hive_id);
   if not FOUND then
     raise exception 'update failed where rack_id=% and hive_id=% ', in_rack_id, in_hive_id;
   end if;
   return 2;
  end if;
 return 0;
end;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
  
DROP FUNCTION public.save_user_action(varchar(16), varchar(32), integer);
CREATE OR REPLACE FUNCTION public.save_user_action(
    name varchar(16),
    action varchar(32),
    max_rows integer
    )
  RETURNS integer AS
$BODY$declare
 n bigint;
 in_mark timestamp without time zone;
begin
  select localtimestamp into in_mark;
  select count(*) from public.user_action where user_name=name into n;
  if not FOUND then
    raise exception 'count failed where user_name=% ', name;
  end if;
  if n < max_rows then
    insert into public.user_action(user_name, action, mark) values (name, action, in_mark);
    if not FOUND then
      raise exception 'insert failed where user_name=% ', name;
    end if;
    return 1;
  else
   update public.user_action set user_name=name, action=action, mark=in_mark where user_name=name and mark=(select min(mark) from public.user_action where user_name=name);
   if not FOUND then
     raise exception 'update failed user_name=% ', name;
   end if;
   return 2;
  end if;
 return 0;
end;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;


-- DROP SCHEMA if exists "log" CASCADE;
CREATE SCHEMA "log";

-- DROP TABLE log.alert;
CREATE TABLE log.alert
(
  mark timestamp without time zone NOT NULL,
  message text NOT NULL
)
WITH (
  OIDS=FALSE
);


-- DROP FUNCTION log.do_real(integer, real, integer, integer, text);

CREATE OR REPLACE FUNCTION log.do_real(
    in_id integer,
    val real,
    row_limit integer,
    tm integer,
    sts text)
  RETURNS integer AS
$BODY$declare
 n bigint;
begin
  select count(*) from log.v_real where id=in_id into n;
  if not FOUND then
    raise exception 'count failed when id was: % ', in_id;
  end if;
  if n<row_limit then
    insert into log.v_real(id, mark, value, status) values (in_id, to_timestamp(tm), val, sts);
    if not FOUND then
      raise exception 'insert failed when id was: % ', in_id;
    end if;
    return 1;
  else
   update log.v_real set mark=to_timestamp(tm), value=val, status=sts where id=in_id and mark=(select min(mark) from log.v_real where id=in_id);
   if not FOUND then
     raise exception 'update failed when id was: % ', in_id;
   end if;
   return 2;
  end if;
 return 0;
end;$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

  
