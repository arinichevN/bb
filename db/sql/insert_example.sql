INSERT OR REPLACE INTO "peer" VALUES
('gwu22_1',49162,'127.0.0.1'),
('gwucn_1',49163,'127.0.0.1'),
('regonf_1',49191,'127.0.0.1'),
('gwu74_1',49173,'127.0.0.1'),
('bb_1',49184,'127.0.0.1');

INSERT OR REPLACE INTO "thread" VALUES
('slow',1,0),
('cooler',0,500),
('flyte',0,500),
('fly',0,1000000);

INSERT OR REPLACE INTO "prog" (id,goal,delta,duty_cycle,open_duty_cycle,close_duty_cycle,interval_read_sec,interval_set_sec) VALUES
(1, 30, 0.5, 0, 1000, 0, 3, 1),
(2, 30, 0.5, 0, 1000, 0, 3, 1),
(3, 30, 0.5, 0, 1000, 0, 3, 1);


INSERT OR REPLACE INTO "channel" (id, description, prog_id, temp_sensor_remote_channel_id, hum_sensor_remote_channel_id, fly_sensor_remote_channel_id, reg_remote_channel_id, flyte_remote_channel_id, cycle_duration_sec, cycle_duration_nsec, save, enable, load ) VALUES
(1,'канал1',1,  1,4,7,10,11,  1,0, 1,1,1),
(2,'канал2',2,  2,5,8,10,11, 1,0, 1,1,1),
(3,'канал3',3,  3,6,9,10,11, 1,0, 1,1,1);

