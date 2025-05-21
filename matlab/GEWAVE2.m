% -----------------------
% 参数设定
% -----------------------
fs = 62500;         % 采样率
f_table = [9000, 10000, 11000, 12000];  % 4FSK 频率表
duration = 0.02;    % 每段信号 20ms（注意：你之前写的单位注释错了）
gap = 0.03;         % 间隔 30ms
t_signal = 0:1/fs:duration-1/fs;
t_gap = 0:1/fs:gap-1/fs;
gap_signal = zeros(1, length(t_gap));

% -----------------------
% 输入 HEX 数据（10字节）
% -----------------------
hex_str = '5A55111111111111111E';  % 输入十六进制字符串
bytes = uint8(sscanf(hex_str, '%2x').');  % 转换为 uint8 数组

% -----------------------
% 构造对应频率信号库
% -----------------------
signal_bank = cell(1, 4);
for i = 1:4
    signal_bank{i} = sin(2 * pi * f_table(i) * t_signal);
end

% -----------------------
% 解析每 2bit → 频率索引
% -----------------------
bitstream = [];
for i = 1:length(bytes)
    byte = bytes(i);
    for j = 3:-1:0
        two_bits = bitand(bitshift(byte, -j*2), 3);  % 提取每2位
        bitstream(end+1) = two_bits + 1;  % 存储频率索引（1~4）
    end
end

% -----------------------
% 构造输出波形
% -----------------------
output = [];
for k = 1:length(bitstream)
    freq_idx = bitstream(k);
    signal = signal_bank{freq_idx};
    output = [output, signal, gap_signal];
end

% -----------------------
% 绘图展示
% -----------------------
% t_total = (0:length(output)-1) / fs;
% figure;
% plot(t_total, output);
% xlabel('时间 (秒)');
% ylabel('幅值');
% title('4FSK 信号波形');
% grid on;

% -----------------------
% 可选播放
% -----------------------
sound(output, fs);
