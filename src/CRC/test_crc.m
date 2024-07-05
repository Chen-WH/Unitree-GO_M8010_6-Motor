% clc;clear;
% 配置串口
% serialPort = 'COM9'; % Windows下的串口名称
% baudRate = 4000000; % 4Mbps

% 创建串口对象
% s = serialport(serialPort, baudRate, 'DataBits', 8, 'StopBits', 1, 'Parity', 'none');

% 准备发送数据
% 发送指令格式：表1中的指令格式，长度为17字节
% 例如：发送一个简单的控制命令
% ID = 1, 模式 = 1 (FOC 闭环), 其他参数根据需要设置
id = 1;
mode = 0;
tau = 0.0;
omega = 0.0;
pos = 0.0;
kp = 0.0;
kd = 0.0;

% 构造发送指令
% 转矩、速度、位置、刚度和阻尼等参数按照文档中的描述进行编码
command = uint8([hex2dec('FE'), hex2dec('EE'), ...
                 mode*16 + id, ...
                 typecast(int16(fix(tau * 256)), 'uint8'), ...
                 typecast(int16(fix(omega / (2 * pi) * 256)), 'uint8'), ...
                 typecast(int32(fix(pos / (2 * pi) * 32768)), 'uint8'), ...
                 typecast(int16(fix(kp * 1280)), 'uint8'), ...
                 typecast(int16(fix(kd * 1280)), 'uint8'), ...
                 0, 0]); % CRC16 计算稍后添加

% 计算并添加 CRC16 校验
crc = generateCRC(command(1:end-2)); % 自定义 CRC 计算函数
command(end-1:end) = typecast(uint16(crc), 'uint8');
% dec2hex(command)
% 发送指令
write(s, command, 'uint8');

% 接收电机状态
% 接收数据格式：表2中的状态格式，长度为16字节
response = read(s, 16, 'uint8');
% response = [hex2dec('FD'), hex2dec('EE'), hex2dec('00'), hex2dec('00'), ...
    % hex2dec('00'), hex2dec('10'), hex2dec('00'), hex2dec('98'), ...
    % hex2dec('CE'), hex2dec('00'), hex2dec('00'), hex2dec('19'), ...
    % hex2dec('78'), hex2dec('0E'), hex2dec('B0'), hex2dec('16')];
% 解析接收到的数据
% 根据文档描述解析各个字段，例如转矩、速度、位置等
tau = double(typecast(uint8(response(4:5)), 'int16')) / 256
omega = double(typecast(uint8(response(6:7)), 'int16')) / 256 * 2 * pi
pos = double(typecast(uint8(response(8:11)), 'int32')) / 32768 * 2 * pi
crc = generateCRC(response(1:end-2)); % 自定义 CRC 计算函数
check = false;
if typecast(uint8(response(end-1:end)), 'uint16') == crc
    check = true;
end
check

id = 1;
mode = 0;
tau = 0.0;
omega = 0.0;
pos = 0.0;
kp = 0.0;
kd = 0.0;

command = uint8([hex2dec('FE'), hex2dec('EE'), ...
                 mode*16 + id, ...
                 typecast(int16(fix(tau * 256)), 'uint8'), ...
                 typecast(int16(fix(omega / (2 * pi) * 256)), 'uint8'), ...
                 typecast(int32(fix(pos / (2 * pi) * 32768)), 'uint8'), ...
                 typecast(int16(fix(kp * 1280)), 'uint8'), ...
                 typecast(int16(fix(kd * 1280)), 'uint8'), ...
                 0, 0]); % CRC16 计算稍后添加

% 计算并添加 CRC16 校验
crc = generateCRC(command(1:end-2)); % 自定义 CRC 计算函数
command(end-1:end) = typecast(uint16(crc), 'uint8');
% 发送指令
write(s, command, 'uint8');
response = read(s, 16, 'uint8');
% 解析接收到的数据
% 根据文档描述解析各个字段，例如转矩、速度、位置等
tau = double(typecast(uint8(response(4:5)), 'int16')) / 256
omega = double(typecast(uint8(response(6:7)), 'int16')) / 256 * 2 * pi
pos = double(typecast(uint8(response(8:11)), 'int32')) / 32768 * 2 * pi
crc = generateCRC(response(1:end-2)); % 自定义 CRC 计算函数
check = false;
if typecast(uint8(response(end-1:end)), 'uint16') == crc
    check = true;
end
check

% 关闭串口
clear s;