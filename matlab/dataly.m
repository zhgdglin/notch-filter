% 读取二进制数据文件（如 .dat 或非标准 .mat）
dataFile = 'relx_10.mat'; % 可修改为你要分析的文件名
fs = 62500; % 采样率，需根据实际情况修改
fid = fopen(dataFile, 'rb');
if fid == -1
    error(['文件未找到: ', dataFile]);
end
data = fread(fid, inf, 'float32');
fclose(fid);

% 简单分析：画时域波形和频谱
figure;
subplot(2,1,1);
plot(data);
title(['时域波形: ', dataFile]);
xlabel('样本点');
ylabel('幅值');

subplot(2,1,2);
N = length(data);
Y = fft(data);
Y = Y(1:floor(N/2)); % 只取正频率部分
f = (0:floor(N/2)-1) * fs / N; % 实际频率坐标
plot(f, abs(Y));
title('幅度谱（正频率）');
xlabel('频率 (Hz)');
ylabel('幅度');