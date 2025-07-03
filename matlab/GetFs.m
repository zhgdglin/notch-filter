
function Fs = GetFs(nSample);
%根据采样率设置参数，获得实际采样率

switch nSample
    case 0
        Fs = 1000;
    case 1
        Fs = 2000;
    case 2
        Fs = 2500;
    case 3
        Fs = 5000;
    case 4
        Fs = 10000;
    case 5
        Fs = 20000;
    case 6
        Fs = 25000;
    case 7
        Fs = 50000;
    case 8
        Fs = 100000;
    otherwise
        Fs = 0;
        warning('Unexpected sample frequence.');
end 


