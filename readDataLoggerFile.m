close all;
clear all;
%Conversion Factor for accelerometer 
%NOTE: This is for the ADXL322, 5V accelerometer
ACC_VOLTS_TO_MS2 = 1/(.75 * 9.81);   % m/s^2 / V

fid = fopen('test.dat', 'r');

% Read the header... this will need to be parsed later.
h_filename = fgetl(fid)
h_timestamp = fgetl(fid)
h_acc_sensor = fgetl(fid)
h_acc_unit = fgetl(fid)
h_sample_rate = fgetl(fid)
h_data_format = fgetl(fid)
h_data_streams = fgetl(fid)

[junk, sample_rate] = strtok(h_sample_rate);
sample_rate = str2num(sample_rate);

[junk, data_format] = strtok(h_data_format);
data_format = lower(strtrim(data_format));

% The number, order and name of these data streams can possibly be pulled from the
% header info, but is hard coded for now.
num_channels = 6;
data = fread(fid, [num_channels,inf], data_format);
fclose(fid);

% Create a time vector.
t = [0:length(data)-1]/sample_rate;

MasterAccLeftx = data(1,:);
MasterAccLefty = data(2,:);
MasterAccLeftz = data(3,:);
MasterAccRightx = data(4,:);
MasterAccRighty = data(5,:);
MasterAccRightz = data(6,:);

% should speciMasterAccLefty left and right or A and B
% cur1 = data(7,:); % subtract the gravity vector
% cur2 = data(8,:);
%acc1 = acc1_V * ACC_VOLTS_TO_MS2;
%acc2 = acc2_V * ACC_VOLTS_TO_MS2;

%cur1 = data(9,:);
%cur2 = data(10,:);

% plot the data.
% figure(1); clf;
% plot(t, MasterAccLeftx, 'r', t, MasterAccLefty, 'g', t, MasterAccLeftz, 'b');
% xlabel('Time (s)')
% ylabel('Master Handle 1 (V)')
% ylim([0 5]);
% 
% figure(2); clf;
% plot(t, MasterAccRightx, 'r', t, MasterAccRighty, 'g', t, MasterAccRightz, 'b');
% xlabel('Time (s)')
% ylabel('Master Handle 2 (V)')
% ylim([0 5]);
% 
% figure(4); clf;
% plot(t, cur1, 'r', t, cur2, 'b');
% xlabel('Time (s)')
% ylabel('Currents (V)')


figure(1);
subplot(2,1,1);
y = data(1,:);
NFFT = 2^12;
Y = fft(y, NFFT, 2);
f = sample_rate/2*linspace(0,1,NFFT/2+1);
%f_max = f(find(Y == max(Y(1:NFFT/2+1))))
plot(f,2*abs(Y(1:NFFT/2+1))) 
z = data(2,:);
NFFT = 2^12;
Z = fft(z, NFFT, 2);
f = sample_rate/2*linspace(0,1,NFFT/2+1);
hold on; 
plot(f,2*abs(Z(1:NFFT/2+1)), 'r') 

% Calculate delay
lag = xcorr(y, z);
delay = length(y) - find(lag == max(lag));

subplot(2,1,2);
plot(t, y, 'b'); hold on;
plot(t(1:end-delay), z((delay+1):end), 'r');
title(['Delay is ' num2str(delay/sample_rate)]);
%plot(t(1:end-1), y(1:end-1)-z(2:end), 'gx');

