% 参数设定
fs = 62500;       % 采样率，62.5 kHz
f = 9000;         % 信号频率，9 kHz
duration = 0.02;  % 每段信号持续时间，20 ms
gap = 0.03;    % 信号之间的间隔时间，30 μs
num_signals = 10; % 生成的信号段数

% 时间轴
t_signal = 0:1/fs:duration-1/fs; % 单段信号的时间轴
t_gap = 0:1/fs:gap-1/fs;         % 间隔信号的时间轴

% 单段信号
signal = sin(2 * pi * f * t_signal);

% 间隔部分（0 信号）
gap_signal = zeros(1, length(t_gap));

% 初始化输出信号
output = [];

% 循环生成信号段和间隔
for i = 1:num_signals
    output = [output, signal, gap_signal];
end

% 绘制生成的信号
t_total = (0:length(output)-1) / fs; % 总时间轴
figure;
plot(t_total, output);
xlabel('时间 (秒)');
ylabel('幅值');
title('9 kHz 单频信号与间隔');
grid on;

% 播放信号（可选）
sound(output, fs);
