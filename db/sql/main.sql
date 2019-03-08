CREATE TABLE "rack" (
    "id" INTEGER PRIMARY KEY,
    "port" INTEGER NOT NULL,
    "ip_addr" TEXT NOT NULL
);
CREATE TABLE "remote_channel"
(
  "id" INTEGER PRIMARY KEY,
  "peer_id" TEXT NOT NULL,
  "channel_id" INTEGER NOT NULL
);

CREATE TABLE "thread"
(
    "id" VARCHAR(8) PRIMARY KEY,
    "cycle_duration_sec" INTEGER NOT NULL,
    "cycle_duration_nsec" INTEGER NOT NULL
);

CREATE TABLE "ds18b20"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "address" TEXT NOT NULL,
    "resolution" INTEGER NOT NULL
);
CREATE TABLE "ds18b20"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "address" TEXT NOT NULL,
    "resolution" INTEGER NOT NULL
);

CREATE TABLE "pwm"
(
    "id" INTEGER PRIMARY KEY,
    "resolution" INTEGER NOT NULL,
    "period_sec" INTEGER NOT NULL,
    "period_nsec" INTEGER NOT NULL,
    "dc_min_sec" INTEGER NOT NULL,
    "dc_min_nsec" INTEGER NOT NULL,
    "dc_max_sec" INTEGER NOT NULL,
    "dc_max_nsec" INTEGER NOT NULL,
);

CREATE TABLE "hive"
(
    "id" INTEGER PRIMARY KEY,
    "fly_sensor_pin" INTEGER NOT NULL,
    "fly_sensor_delay_sec" INTEGER NOT NULL,
    "fly_sensor_delay_nsec" INTEGER NOT NULL,
    "fly_sensor_read_interval_sec" INTEGER NOT NULL,
    "fly_sensor_read_interval_nsec" INTEGER NOT NULL,
    "presence_sensor_pin" INTEGER NOT NULL
);
CREATE TABLE "note"
(
    "id" VARCHAR(8) PRIMARY KEY,
    "frequency" INTEGER NOT NULL
);
CREATE TABLE "sound"
(
    "id" INTEGER PRIMARY KEY,
    "sequence" INTEGER NOT NULL,
    "note_id" VARCHAR(8) NOT NULL,
    "duration" INTEGER NOT NULL
);
CREATE TABLE "prog" (
    "id" INTEGER PRIMARY KEY,
    "goal" REAL NOT NULL,
	"duty_cycle" REAL NOT NULL,
    "open_duty_cycle" REAL NOT NULL,
	"close_duty_cycle" REAL NOT NULL,
	"interval_log_sec" INTEGER NOT NULL,
	"interval_replace_sec" INTEGER NOT NULL,
    "interval_disable_sec" INTEGER NOT NULL,
	"interval_set_sec" INTEGER NOT NULL
);
CREATE TABLE "channel" (
    "id" INTEGER PRIMARY KEY,
    "prog_id" INTEGER NOT NULL,
    "reg_remote_channel_id" INTEGER NOT NULL,
    "flyte_remote_channel_id" INTEGER NOT NULL,
    "flyte_remote_channel_id" INTEGER NOT NULL,
    "sound_sensor_remote_channel_id" INTEGER NOT NULL,
    "cycle_duration_sec" INTEGER NOT NULL,
    "cycle_duration_nsec" INTEGER NOT NULL,
    "save" INTEGER NOT NULL,
    "enable" INTEGER NOT NULL,
    "load" INTEGER NOT NULL
);
CREATE TABLE "channel_hive"
(
    "channel_id" INTEGER PRIMARY KEY,
    "hive_id" INTEGER NOT NULL
);
