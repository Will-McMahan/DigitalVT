% Perhaps write these to the top of the binary file in C++ and read them in here.
num_channels = 9;
sample_rate = 2000;
g = 9.81 %m/s

%data = dlmread('test.txt', '\t');
fid = fopen('test.txt', 'r');
data = fread(fid, [9,inf], 'float');
fclose(fid);

t = [0:length(data)-1]/sample_rate;

Fx = data(1,:);
Fy = data(2,:);
Fz = data(3,:);
Tx = data(4,:);
Ty = data(5,:);
Tz = data(6,:);
acc1 = data(7,:) - mean(data(7,:));
acc2 = data(8,:) - mean(data(8,:));
sync = data(9,:);

%figure(1); clf;
%plot(t(1:sample_rate+10), acc1(1:sample_rate+10), '-x');
%hold on;
%plot(t(1:sample_rate+10), sin(2*pi*416*t(1:sample_rate+10)), 'r-o');

figure(2); clf;
[X, freq] = myFFT(acc1, sample_rate, 0); plot(freq, X);

figure(3); clf;
plot(Fz/9.81);

figure(4); clf;
plot(sync);



