CREATE TABLE "peer" (
    "id" TEXT NOT NULL,
    "port" INTEGER NOT NULL,
    "ip_addr" TEXT NOT NULL
);
CREATE TABLE "remote_channel"
(
  "id" INTEGER PRIMARY KEY,
  "peer_id" TEXT NOT NULL,
  "channel_id" INTEGER NOT NULL
);
CREATE TABLE "prog" (
    "id" INTEGER PRIMARY KEY,
    "goal" REAL NOT NULL,
    "delta" REAL NOT NULL,
	"duty_cycle" REAL NOT NULL,
    "open_duty_cycle" REAL NOT NULL,
	"close_duty_cycle" REAL NOT NULL,
	"interval_read_sec" INTEGER NOT NULL,
	"interval_set_sec" INTEGER NOT NULL
);
CREATE TABLE "channel" (
    "id" INTEGER PRIMARY KEY,
    "description" TEXT NOT NULL,
    "prog_id" INTEGER NOT NULL,
    "temp_sensor_remote_channel_id" INTEGER NOT NULL,
	"hum_sensor_remote_channel_id" INTEGER NOT NULL,
	"fly_sensor_remote_channel_id" INTEGER NOT NULL,
    "reg_remote_channel_id" INTEGER NOT NULL,
    "flyte_remote_channel_id" INTEGER NOT NULL,
    "cycle_duration_sec" INTEGER NOT NULL,
    "cycle_duration_nsec" INTEGER NOT NULL,
    "save" INTEGER NOT NULL,
    "enable" INTEGER NOT NULL,
    "load" INTEGER NOT NULL
);
