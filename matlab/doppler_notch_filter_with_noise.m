function multi_channel_adaptive_notch_filter
    clc;
    clear all;
    close all;
    % 参数设置
    fs = 62500;  % 采样频率为62.5 kHz
    frequencies = [9000, 10000, 11000, 12000]; % 频率数组
    t_signal = 0.02; % 每个信号持续时间 20 ms
    t_gap = 0.03;    % 信号之间的间隔时间 30 ms
    A = 1;       % 输入信号和参考信号的幅度
    mu = 0.001;  % 学习步长，针对较高频率进行了调整
    
    % 时间向量生成
    t = 0:1/fs:(t_signal+t_gap)*length(frequencies)-1/fs;

    d = zeros(size(t));

    % 生成信号
    for i = 1:length(frequencies)
        start_idx = (i-1)*(t_signal+t_gap)*fs + 1;
        end_idx = start_idx + t_signal*fs - 1;
        d(start_idx:end_idx) = sin(2*pi*frequencies(i)*t(1:t_signal*fs));
    end
    
    
    % 生成噪声并加到原始信号上
    SNR = -35;  % 期望的信噪比（dB）
    signal_power = var(d);  % 信号功率
    noise_power = signal_power / (10^(SNR/10));  % 噪声功率计算
    noise = sqrt(noise_power) * randn(size(d));  % 生成高斯白噪声
    d_noisy = d + noise;  % 向信号添加噪声
    % 生成正交参考输入信号 
    xc1 = A * cos(2*pi*9000*t);
    xs1 = A * sin(2*pi*9000*t);
    xc2 = A * cos(2*pi*10000*t);
    xs2 = A * sin(2*pi*10000*t);
    xc3 = A * cos(2*pi*11000*t);
    xs3 = A * sin(2*pi*11000*t);

    % 初始化滤波器权值
    uc1 = 0.1;
    us1 = 0.1;
    uc2 = 0.1;
    us2 = 0.1;
    uc3 = 0.1;
    us3 = 0.1;

    % 输出信号初始化
    y1 = zeros(size(t));
    y2 = zeros(size(t));
    y3 = zeros(size(t));
    e = zeros(size(t));

    % 并行多通道自适应陷波滤波器
    for k = 2:length(t)
        % 第一个单频自适应陷波滤波器 (9 kHz)
        y1(k) = uc1 * xc1(k) + us1 * xs1(k);
        %e1 = d_noisy(k) - y1(k);
        e1 = d(k) - y1(k);
        uc1 = uc1 + mu * e1 * xc1(k);
        us1 = us1 + mu * e1 * xs1(k);

        % 第二个单频自适应陷波滤波器 (10 kHz)
        y2(k) = uc2 * xc2(k) + us2 * xs2(k);
        %e2 = d_noisy(k) - y2(k);
        e2 = d(k) - y2(k);
        uc2 = uc2 + mu * e2 * xc2(k);
        us2 = us2 + mu * e2 * xs2(k);

        % 第三个单频自适应陷波滤波器 (11 kHz)
        y3(k) = uc3 * xc3(k) + us3 * xs3(k);
        %e3 = d_noisy(k) - y3(k);
        e3 = d(k) - y3(k);
        uc3 = uc3 + mu * e3 * xc3(k);
        us3 = us3 + mu * e3 * xs3(k);

        % 残差输出（总输出）
        e(k) = y1(k) + y2(k) + y3(k);
    end

    % 绘制时间域信号
    figure;
    subplot(3,1,1)
    plot(t, d);
    title('原始信号');
    xlabel('时间 (秒)');
    ylabel('幅度');
    
    subplot(312)
    plot(t, d_noisy);
    title('原始信号+噪声');
    xlabel('时间 (秒)');
    ylabel('幅度');

    subplot(3,1,3)
    plot(t, e);
    title('滤波后信号');
    xlabel('时间 (秒)');
    ylabel('幅度');

    % 计算频谱
    N = length(t);
    D_f = fft(d)/N;
    Dn_f = fft(d_noisy)/N;
    E_f = fft(e)/N;
    f = (0:N-1)*(fs/N); % 频率向量

    % 绘制原始信号的频谱
    figure;
    subplot(3,1,1)
    plot(f(1:N/2), 2*abs(D_f(1:N/2)), 'r');
    title('原始信号的频谱');
    xlabel('频率 (Hz)');
    ylabel('幅度');
    
    

    % 绘制原始信号+噪声的频谱
    subplot(3,1,2)
    plot(f(1:N/2), 2*abs(Dn_f(1:N/2)), 'r');
    title('原始信号+噪声的频谱');
    xlabel('频率 (Hz)');
    ylabel('幅度');

    % 绘制滤波后信号的频谱
    subplot(3,1,3)
    plot(f(1:N/2), 2*abs(E_f(1:N/2)), 'b');
    title('滤波后信号的频谱');
    xlabel('频率 (Hz)');
    ylabel('幅度');
    
    input_signal = d;  % 含噪声的输入信号
    output_signal = y3;  % 滤波后的输出信号

%     % 计算输入信号的瞬时频率
%     input_inst_phase = unwrap(angle(hilbert(input_signal)));
%     input_inst_freq = diff(input_inst_phase) * fs / (2 * pi);
% 
%     % 计算输出信号的瞬时频率
%     output_inst_phase = unwrap(angle(hilbert(output_signal)));
%     output_inst_freq = diff(output_inst_phase) * fs / (2 * pi);
% 
%     % 绘制瞬时频率图
%     figure;
%     subplot(2,1,1);
%     plot(t(2:end), input_inst_freq / 1e3, 'r');
%     title('输入信号的瞬时频率');
%     xlabel('时间 (秒)');
%     ylabel('频率 (kHz)');
%     grid on;
% 
%     subplot(2,1,2);
%     plot(t(2:end), output_inst_freq / 1e3, 'b');
%     title('输出信号的瞬时频率');
%     xlabel('时间 (秒)');
%     ylabel('频率 (kHz)');
%     grid on;
% 
%     sgtitle('输入信号与输出信号的瞬时频率对比');

% 设置参数
    %fs = 62500;  % 采样频率
    window_length = 256;  % STFT 窗口长度
    overlap = round(0.75 * window_length);  % 窗口重叠
    nfft = 1024;  % FFT 点数

    % 生成时间向量
    %t = 0:1/fs:(t_signal + t_gap) * length(frequencies) - 1/fs;

    % 输入信号（含噪声）
    input_signal = d_noisy;

    % 输出信号（滤波后的输出信号）
    output_signal = y3;

    % 计算输入信号的 STFT
    [~, f, t_stft, ps_input] = spectrogram(input_signal, window_length, overlap, nfft, fs);

    % 通过频谱质心法计算输入信号的瞬时频率
    input_inst_freq = sum(f .* abs(ps_input), 1) ./ sum(abs(ps_input), 1);

    % 计算输出信号的 STFT
    [~, ~, ~, ps_output] = spectrogram(output_signal, window_length, overlap, nfft, fs);

    % 通过频谱质心法计算输出信号的瞬时频率
    output_inst_freq = sum(f .* abs(ps_output), 1) ./ sum(abs(ps_output), 1);

    % 绘制瞬时频率图
    figure;
    subplot(2,1,1);
    plot(t_stft, input_inst_freq / 1e3, 'r');
    title('输入信号的瞬时频率 (STFT)');
    xlabel('时间 (秒)');
    ylabel('频率 (kHz)');
    grid on;

    subplot(2,1,2);
    plot(t_stft, output_inst_freq / 1e3, 'b');
    title('输出信号的瞬时频率 (STFT)');
    xlabel('时间 (秒)');
    ylabel('频率 (kHz)');
    grid on;

    sgtitle('输入信号与输出信号的瞬时频率对比 (STFT)');


end
