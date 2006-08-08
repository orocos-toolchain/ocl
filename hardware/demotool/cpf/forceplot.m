clear all; close all;

input = load('input_new.txt');

sample_freq = 100;
desired_freq = 20;
resample = 20;
butter_factor = desired_freq / sample_freq;
resample_factor = resample / sample_freq;

[B A] = butter(7, butter_factor, 'low');
[size_input tmp] = size(input);
counter = 1;

time = input(:,counter);
time_filter = time(1:1/resample_factor:size_input,:);
counter = counter +1;

Wrench_fs_fs = input(:,counter:counter+5);
Wrench_fs_fs_filter = lowpass(B, A, resample_factor, Wrench_fs_fs);
counter = counter +6;

Wrench_world_world = input(:,counter:counter+5);
Wrench_world_world_filter = lowpass(B, A, resample_factor, Wrench_world_world);
counter = counter +6;

Wrench_obj_obj = input(:,counter:counter+5);
Wrench_obj_obj_filter = lowpass(B, A, resample_factor, Wrench_obj_obj);
counter = counter +6;

Twist_world_world = input(:,counter:counter+5);
Twist_world_world_filter = lowpass(B, A, resample_factor, Twist_world_world);
counter = counter +6;

Twist_obj_world = input(:,counter:counter+5);
Twist_obj_world_filter = lowpass(B, A, resample_factor, Twist_obj_world);
counter = counter +6;

Frame_world_obj = input(:,counter:counter+11);
Frame_world_obj_filter = lowpass(B, A, resample_factor, Frame_world_obj);
counter = counter +12;

leds = input(:,counter);
leds_filter = leds(1:1/resample_factor:size_input,:);


filterdata = [time_filter Frame_world_obj_filter Wrench_world_world_filter Twist_world_world_filter leds_filter];

save -ASCII filterdata.txt filterdata;

figure;
plot(time, Frame_world_obj); title('Frame world obj');
figure;
plot(time_filter, Frame_world_obj_filter); title('Frame world obj filter');

figure;
plot(time, Twist_world_world); title('Twist world world');
figure;
plot(time_filter, Twist_world_world_filter); title('Twist world world filter');

figure;
plot(time, Wrench_obj_obj); title('Wrench obj obj');
figure;
plot(time_filter, Wrench_obj_obj_filter); title('Wrench obj obj filter');

figure;
plot(time, Wrench_world_world); title('Wrench world world');
figure;
plot(time_filter, Wrench_world_world_filter); title('Wrench world world filter');

figure;
plot(time, leds); title('Num Leds');


