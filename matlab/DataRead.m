%% 打开数据文件
% fid = fopen('G:\宝铭\1\P20191226\P28_102354.dat');
fs=62500;
% N=4096;
% t=0:1/fs:(N-1)/fs; %时域采样点
% f2=(0:(N-1))*fs/N; %频域采样点
fid = fopen('D:\software\HydroWatch\logFolder\自容式水听器_1#\P20250628\P28_105351.dat');
chHeader = fread(fid, 28, 'bit8'); 
nSample = fread(fid, 1, 'int32');
nAmpGain = fread(fid, 1, 'int32');
nFilter = fread(fid, 1, 'int32');
nUnit = fread(fid, 1, 'int32');
fSensitivity = fread(fid, 1, 'float32');
nDataType = fread(fid, 1, 'int32');
chHeader2 = fread(fid, 512-52, 'bit8');
Fs  = GetFs(nSample);
Nlen = 300000;                  %数据长度 需要显示的数据长度   
Vae = 1/2^24*8.192;             %24bit整型数转换为电压的转换系数
Data = fread(fid, inf, 'bit24');
Data = Data*Vae;
N1=length(Data);
fclose(fid);
figure
% plot(Data);

x2=resample(Data,fs,Fs);
N2=length(x2);
% hold on
% plot(x2);
bandpass = fir1(512, 2*[8e3 14e3]/fs);
bandpass_Sig = filter(bandpass,1,x2);
plot(bandpass_Sig);

% 提取指定区间
start_idx = round(5.5e6);
end_idx = round(5.8e6);
if end_idx > length(bandpass_Sig)
    end_idx = length(bandpass_Sig);
end
bandpass_Sig_section = bandpass_Sig(start_idx:end_idx);

% 保存提取的数据到dat文件（以二进制方式保存为float32）
fid_out = fopen('deck_air_20.mat', 'wb');
fwrite(fid_out, bandpass_Sig_section, 'float32');
fclose(fid_out);







