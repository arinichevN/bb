CREATE TABLE "rack" (
    "id" INTEGER PRIMARY KEY,
    "port" INTEGER NOT NULL,
    "ip_addr" TEXT NOT NULL,
    "ds18b20_id" INTEGER NOT NULL,
    "flyte_pwm_id" INTEGER NOT NULL,
    "buzzer_id" INTEGER NOT NULL,
    "" INTEGER NOT NULL,
    "" INTEGER NOT NULL,
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
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL
);

CREATE TABLE "buzzer"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "delay_s" INTEGER NOT NULL,
    "delay_ns" INTEGER NOT NULL,
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL
);
)
CREATE TABLE "ds18b20"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "address" TEXT NOT NULL,
    "resolution" INTEGER NOT NULL
);

CREATE TABLE "pwm_device"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "pwm_id" INTEGER NOT NULL
);

CREATE TABLE "pwm"
(
    "id" INTEGER PRIMARY KEY,
    "resolution" INTEGER NOT NULL,
    "period_s" INTEGER NOT NULL,
    "period_ns" INTEGER NOT NULL,
    "dc_min_s" INTEGER NOT NULL,
    "dc_min_ns" INTEGER NOT NULL,
    "dc_max_s" INTEGER NOT NULL,
    "dc_max_ns" INTEGER NOT NULL,
);

CREATE TABLE "pid_regulator"
(
  "id" INTEGER PRIMARY KEY,
  "kp" REAL NOT NULL,
  "ki" REAL NOT NULL,
  "kd" REAL NOT NULL,
  "min_output" REAL NOT NULL,
  "max_output" REAL NOT NULL,
  "goal" REAL NOT NULL
);

CREATE TABLE "presence"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "delay_disable_s" INTEGER NOT NULL,
    "interval_update_s" INTEGER NOT NULL,
    "max_rows" INTEGER NOT NULL
);

CREATE TABLE "logger"
(
    "id" INTEGER PRIMARY KEY,
    "interval_s" INTEGER NOT NULL,
    "max_rows" INTEGER NOT NULL
);

CREATE TABLE "fly_counter"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "delay_ms" INTEGER NOT NULL,
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL
);

CREATE TABLE "flyte"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "pwm_id" INTEGER NOT NULL,
    "open_duty_cycle" REAL NOT NULL,
    "close_duty_cycle" REAL NOT NULL,
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL
);

CREATE TABLE "cooler"
(
    "id" INTEGER PRIMARY KEY,
    "pin" INTEGER NOT NULL,
    "pwm_id" INTEGER NOT NULL,
    "ds18b20_id" INTEGER NOT NULL,
    "pid_regulator_id" INTEGER NOT NULL,
    "interval_s" INTEGER NOT NULL,
    "interval_ns" INTEGER NOT NULL,
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL
);)

CREATE TABLE "hive"
(
    "id" INTEGER PRIMARY KEY,
    "fly_sensor_pin" INTEGER NOT NULL,
    "fly_sensor_delay_s" INTEGER NOT NULL,
    "fly_sensor_delay_ns" INTEGER NOT NULL,
    "fly_sensor_read_interval_s" INTEGER NOT NULL,
    "fly_sensor_read_interval_ns" INTEGER NOT NULL,
    "presence_sensor_pin" INTEGER NOT NULL
);
CREATE TABLE "note"
(
    "id" TEXT PRIMARY KEY,
    "frequency" INTEGER NOT NULL
);
CREATE TABLE "sound"
(
    "id" INTEGER PRIMARY KEY,
    "sequence" INTEGER NOT NULL,
    "note_id" TEXT NOT NULL,
    "duration" INTEGER NOT NULL
);
CREATE TABLE "prog" (
    "id" INTEGER PRIMARY KEY,
    "goal" REAL NOT NULL,
	"duty_cycle" REAL NOT NULL,
    "open_duty_cycle" REAL NOT NULL,
	"close_duty_cycle" REAL NOT NULL,
	"interval_log_s" INTEGER NOT NULL,
	"interval_replace_s" INTEGER NOT NULL,
    "interval_disable_s" INTEGER NOT NULL,
	"interval_set_s" INTEGER NOT NULL
);
CREATE TABLE "channel" (
    "id" INTEGER PRIMARY KEY,
    "prog_id" INTEGER NOT NULL,
    "reg_remote_channel_id" INTEGER NOT NULL,
    "flyte_remote_channel_id" INTEGER NOT NULL,
    "flyte_remote_channel_id" INTEGER NOT NULL,
    "sound_sensor_remote_channel_id" INTEGER NOT NULL,
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL,
    "save" INTEGER NOT NULL,
    "enable" INTEGER NOT NULL,
    "load" INTEGER NOT NULL
);
CREATE TABLE "channel_hive"
(
    "channel_id" INTEGER PRIMARY KEY,
    "hive_id" INTEGER NOT NULL
);
