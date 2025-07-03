clc
clear
[file,path,indx] = uigetfile('G:\502噪声\1\P20191226\*.dat');
if isequal(file,0)
   disp('User selected Cancel')
else
   disp(['User selected ', fullfile(path, file),' and filter index: ', num2str(indx)])
end
filename = fullfile(path, file);
fid = fopen(filename);
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
Data = fread(fid, inf, 'bit24');
fclose(fid);
plot(Data)

save Data Data

