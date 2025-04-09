clc; clear all; close all;

fs = 62500;   % 采样率  62.5K
f1 = 9e3;    %信号频率
f2 = 9.5e3;    %信号频率
N = 100;    %数据量
t = 0 : 1/fs :(N-1)/fs;
data_bit = 16-1;  % 数据位数 16位  有符号


y1_sin = sin (2*pi*f1*t);
y1_sin = round(y1_sin * (2^data_bit-1));  
y1_cos = cos (2*pi*f1*t);
y1_cos = round(y1_cos * (2^data_bit-1));

y2_sin = sin (2*pi*f2*t);
y2_sin = round(y2_sin * (2^data_bit-1));
y2_cos = cos (2*pi*f2*t);
y2_cos = round(y2_cos * (2^data_bit-1));

% 存入txt文件
% ftxt = fopen('ddradta.txt', 'wt');   
% fprintf(ftxt,'%d\n',y,'int16');    
% fclose(ftxt);

figure
subplot(211); plot(t,y1_sin);
subplot(212); plot(t,y1_cos);

figure
subplot(211); plot(t,y2_sin);
subplot(212); plot(t,y2_cos);