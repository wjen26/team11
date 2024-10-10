clear; clc; close all;

s=serialport('/dev/tty.usbserial-0001',9600);
fopen(s);

configureTerminator(s, "LF");  % LF used as the terminator

% Create a figure for the temperature image
figure;
hImage = imagesc(zeros(64, 64)); % Initialize with a 64x64 matrix
colorbar; % Show color scale
colormap(jet); % Set colormap (you can choose other colormaps)
axis equal tight; % Set equal scaling for axes
clim([15 25]);

% Main loop to continuously update the image
while true
    % Read data from the serial port
    dataString = readline(s); % Read until newline
    dataVector = str2double(split(dataString, ',')); % Convert to numeric vector
    
    % Check if we received 64 temperatures
    if length(dataVector) == 64
        % Reshape the vector into an 8x8 matrix
        tempMatrix = reshape(dataVector, [8, 8]);

        % Resize the image to 64x64 using bicubic interpolation
        resizedTempMatrix = imresize(tempMatrix, [64, 64], 'bicubic'); % Resize to 64x64

        % Update the image data
        set(hImage, 'CData', resizedTempMatrix); % Update the image data
        drawnow; % Update the figure window
    else
        disp('Error: Did not receive 64 temperature values.');
    end
end

% Close the serial port when done (this code won't execute in this loop)
% clear s; 