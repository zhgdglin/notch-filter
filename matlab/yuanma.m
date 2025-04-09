clear all,close all,clc;
%%构造加入噪声与多普勒效应的CW信号
fs=50000;
fff=zeros(7,7);
for kk=1:2
% v=[-15 -10 -5 0 5 10 15];%%定义相对运动时速度为正值
v=[0];
for ii=1:length(v)
c=1500;
f0=7000;
f1(ii)=((c+v(ii))/c)*f0;
t_cw0=0.02;
% t_cw1=(c/(c+v))*t_cw0;%这是正确的模型
t0=0:1/fs:t_cw0;
% t1=0:1/fs:t_cw1;
sig0=sin(2.*pi*f0*t0);
% snr=[-15 -10 -5 0 5 10 15];
snr=[40];
for jj=1:length(snr)
sig1(ii,:)=sin(2.*pi*f1(ii)*t0);%%为了调试方便在此暂时忽略多普勒效应产生的时域影响采用t0
% figure(1),plot(t0,sig0);
% title('原始信号'),xlabel('时间/s'),ylabel('幅值/v'),grid on
% figure(2),plot(t0,sig1);
% title('加入多普勒效应的信号'),xlabel('时间/s'),ylabel('幅值/v'),grid on
%test环节%
% f_test=5020;
% sig_test=sin(2.*pi*f_test*t0);
%给信号加噪声
sig_power=sum(sig1(ii,:).*sig1(ii,:))/length(t0);
std_noise=(sig_power/(10^(snr(jj)/20)))^0.5;
% sig0=sig0+std_noise*randn(size(sig0));

% sig1(ii,:)=sig1(ii,:)+std_noise*randn(size(t0));
% sig_test=sig_test+std_noise*randn(size(sig_test));
%使用notch滤波器进行测频
temp1=0;%调试用
temp2=0;%调试用
f=zeros(1,length(t0));
x=sig1(ii,:);
rc=sin(2*pi*5000*t0);
rs=cos(2*pi*5000*t0);
u=0.02;%%步长太短的话不能充分利用已有信息收敛速度较慢，步长太长的话迭代噪声太大
wc = zeros(1,length(t0)+1);
ws = zeros(1,length(t0)+1);
for k = 1:length(t0)
    y(k) = wc(k)*rc(k) + ws(k)*rs(k);
    e(k)= x(k) - y(k);
    wc(k+1) = wc(k) + u*e(k)*rc(k);
    ws(k+1) = ws(k) + u*e(k)*rs(k);
    if wc(k)==0|wc(k+1)==0%%剔除分母为0的点
        temp1=temp1+1;
    else
        f(k)=f0+fs*(atan(ws(k+1)/wc(k+1))-atan(ws(k)/wc(k)))/(2*pi);
        if abs(atan(ws(k+1)/wc(k+1))-atan(ws(k)/wc(k)))>=pi
            temp2=temp2+1;%检测梁论文中间的一个建议
        end
    end
    if abs(f(k))>=7000|abs(f(k))<=3000
        f(k)=f0;
    end
%     if k==180%%对现有模型改进的尝试
%        ff1=mean(f(100:180));
%        rs=sin(2*pi*ff1*t0);
%        rc=cos(2*pi*ff1*t0);
%     elseif k==330
%        ff2=mean(f(250:330));
%        rs=sin(2*pi*ff2*t0);
%        rc=cos(2*pi*ff2*t0);
%     end
end
figure(3),plot(t0,y);
figure(4),plot(t0,f);
ff(ii,jj)=mean(f(100:end));%%稍后琢磨一下怎么样平滑
end
end
fff=fff+ff;
end
ffff=fff/200;
%%%大多普勒条件下该算法会出现很大的偏差（相对运动在9米每秒时出现大的偏差）
%%%原因出在参考信号的失配，以至于不能使用，下一步进行理论推导找出原因，进一步进行改进
%%%改进时可以考虑参考信号的选择问题
%%%噪声对频率估计的影响也很大，具体进行定量的仿真分析
%%% notch滤波器迭代时间越长测得的频率就越准确，同步头时间越长估计就越准确



