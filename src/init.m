clc;clear;
% 配置串口
serialPort = ["COM9", "COM2"]; % Windows下的串口名称
offset = [5.79, 5.84];
baudRate = 4000000; % 4Mbps
id = [0, 1];

for i = 1:length(id)
    % 创建串口对象
    s = serialport(serialPort(i), baudRate, 'DataBits', 8, 'StopBits', 1, 'Parity', 'none');

    % 发送数据获取当前角度值
    command = uint8([hex2dec('FE'), hex2dec('EE'), ...
        id(i), ...
        typecast(int16(fix(0.0 * 256)), 'uint8'), ...
        typecast(int16(fix(0.0 / (2 * pi) * 256)), 'uint8'), ...
        typecast(int32(fix(0.0 / (2 * pi) * 32768)), 'uint8'), ...
        typecast(int16(fix(0.0 * 1280)), 'uint8'), ...
        typecast(int16(fix(0.0 * 1280)), 'uint8'), ...
        0, 0]);
    crc = crc_ccitt(command(1:end-2));
    command(end-1:end) = typecast(uint16(crc), 'uint8');
    count = 0;
    while true
        write(s, command, 'uint8');
        response = uint8(read(s, 16, 'uint8'));
        crc = crc_ccitt(response(1:end-2));
        pos = double(typecast(response(8:11), 'int32')) / 32768 * 2 * pi;
        if typecast(response(end-1:end), 'uint16') == crc
            break;
        end
        count = count + 1;
        pause(0.001);
        if count > 10000
            error("电机没有返回消息");
        end
    end

    % 电机位置归零
    pos_d = linspace(pos, offset(i), 3000);
    for j = 1:length(pos_d)
        command = uint8([hex2dec('FE'), hex2dec('EE'), ...
            16 + id(i), ...
            typecast(int16(fix(0.0 * 256)), 'uint8'), ...
            typecast(int16(fix(0.0 / (2 * pi) * 256)), 'uint8'), ...
            typecast(int32(fix(pos_d(j) / (2 * pi) * 32768)), 'uint8'), ...
            typecast(int16(fix(1.0 * 1280)), 'uint8'), ...
            typecast(int16(fix(0.0 * 1280)), 'uint8'), ...
            0, 0]);
        crc = crc_ccitt(command(1:end-2));
        command(end-1:end) = typecast(uint16(crc), 'uint8');
        write(s, command, 'uint8');
        pause(0.001);
    end

    % 关闭串口
    clear s;
end
fprintf("电机归零完成\n");