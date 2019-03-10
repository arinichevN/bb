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
);))
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

CREATE TABLE "pid"
(
  "id" INTEGER PRIMARY KEY,
  "kp" REAL NOT NULL,
  "ki" REAL NOT NULL,
  "kd" REAL NOT NULL,
  "output_min" REAL NOT NULL,
  "output_max" REAL NOT NULL
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
    "pid_id" INTEGER NOT NULL,
    "goal" REAL NOT NULL,
    "interval_s" INTEGER NOT NULL,
    "interval_ns" INTEGER NOT NULL,
    "cycle_duration_s" INTEGER NOT NULL,
    "cycle_duration_ns" INTEGER NOT NULL
);)

CREATE TABLE "rack_hive"
(
    "rack_id" INTEGER PRIMARY KEY,
    "id" INTEGER PRIMARY KEY,
    "fly_counter_id" INTEGER NOT NULL,
    "fly_logger_id" INTEGER NOT NULL,
    "temp_logger_id" INTEGER NOT NULL,
    "presence_id" INTEGER NOT NULL
);
ALTER TABLE "rack_hive" ADD CONSTRAINT "rh_pkey" PRIMARY KEY ("rack_id", "id");

CREATE TABLE "rack" (
    "id" INTEGER PRIMARY KEY,
    "flyte_id" INTEGER NOT NULL,
    "cooler_id" INTEGER NOT NULL,
    "buzzer_id" INTEGER NOT NULL,
    "door_pin" INTEGER NOT NULL
);



