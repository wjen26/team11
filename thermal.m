clear; clc; close all;
clear s;

% Setup the serial port
s = serialport('COM5', 9600);
configureTerminator(s, "LF");

allData = [];  % Initialize an empty array to store numeric data

% Create a figure window before the loop starts
figure;
h = imagesc(zeros(8, 8));  % Initialize with a blank 8x8 matrix
colormap('hot');  
colorbar;  % Show the colorbar



% Set the fixed color axis range between 15 and 35
%caxis([23, 30]);  % Keep the color scale fixed between 15 and 35 for temperature value

%initialize old blob count and new blob count
blob_count = 0;
old_blob_count = 0;
%create threshold temp
Tmax = 35;
row_1and2_old = [];
last_rows_old = [];
top_8_old = [];
top_8_last_old = [];
%initialize num_features
old_num_features = 0;

%initialize count
count = 0;

% Initialize calibration step
need_cal = true;
cal_count = 0;
Tamb = 0; calibration_matrices = {}; calibration_matrices2 = {}; TH1 = 0; TH2 = 0; binary_TH = 16; ch = 15;

matrices = {};
mid_value = [];
binary_vec = [];

% Loop to read data
while true

    dataString = strtrim(readline(s));  % Read and trim whitespace from data
    
    % Skip empty lines entirely
    if isempty(dataString)
        continue;
    end
    
    % Replace the arrows (→) with spaces to separate the numbers
    cleanedDataString = regexprep(dataString, '→', ' ');  
    
    % Split the data by spaces (or tabs) into an array of strings
    dataArray = strsplit(cleanedDataString);
    
    % Initialize an array to hold numeric values
    dataNums = NaN(size(dataArray));
    
    % Convert each element to a number
    for i = 1:length(dataArray)
        element = strtrim(dataArray{i});  % Trim each element for safety
        if isempty(element)
            continue;  % Skip empty strings
        end
        
        numValue = str2double(element);  % Try converting to a number
        
        if ~isnan(numValue)
            dataNums(i) = numValue;  % Store valid numeric data
        end
    end
    
    % Filter out NaNs and check if any valid data was received
    validDataNums = dataNums(~isnan(dataNums));
    
    if ~isempty(validDataNums)
        % Append valid numbers to allData
        allData = [allData, validDataNums];  

        % Check if we have accumulated 64 data points (8x8 matrix)
        if length(allData) >= 64
            
            % Take the first 64 valid data points and reshape into an 8x8 matrix
            reshapedData = reshape(allData(1:64), 8, 8);
            %disp(reshapedData)
            if cal_count > 30
                need_cal = false;
                disp("calibrated")
            end

            if need_cal == true   
                if cal_count == 0
                    disp("starting calibration")
                    % Remove the first 64 points from allData if you want to continue reading
                    allData = allData(65:end);
                end
                if cal_count == 1% set Tamb
                    disp("step 1")
                    Tamb = cal_step_1(reshapedData);
                    % Remove the first 64 points from allData if you want to continue reading
                    allData = allData(65:end);
                end
            % set calibration_matrices
                if cal_count >= 2 && cal_count <= 11
                 disp("step 2")
                 calibration_matrices{end+1} = cal_step_2(reshapedData,ch,Tamb);
                 % Remove the first 64 points from allData if you want to continue reading
                 allData = allData(65:end);
                end 
            
                if cal_count == 12
                    disp("step 3")
                    TH1 = cal_step_3(calibration_matrices);
                    
                    % Remove the first 64 points from allData if you want to continue reading
                    allData = allData(65:end);
                end
                
                if cal_count > 12 && cal_count < 30
                    reshapedData = real_temp(reshapedData,ch,Tamb);
                    reshapedData = rot90(reshapedData,-1)
                    disp(reshapedData)
                    if ~any(reshapedData(1:2,:) > 35)
                        disp("step 4, need someone in view")
                        cal_count = cal_count - 1;
                    else
                        disp("identified someone in view")
                        calibration_matrices2{end+1} = reshapedData;
                    end
                    allData = allData(65:end);
                end
                
                if cal_count == 30
                    disp("step 5")
                    TH2 = cal_step_5(calibration_matrices2);
                end     
            end
            
            cal_count = cal_count + 1;
            
            reshapedData = rot90(reshapedData,-1);
            
            
            if need_cal == false
            
            reshapedData = real_temp(reshapedData,ch,Tamb);
            %reshapedData = rot90(reshapedData,-1)
            matrices{end+1} = reshapedData;
            mid_value(end+1) = mean(mean(reshapedData(4:5,4:5)));
            
            % Calculate the average of the 8x8 matrix
            avgValue = mean(reshapedData(:));  % Compute the average of all values in the matrix
            disp("avg")
            disp(avgValue)
            maxValue = max(reshapedData(:));
            binary_map = reshapedData > TH2;
            weighted_binary = binary_map.*reshapedData;
            binary_vec(end+1) = sum(binary_map(:));
            
            
            %if avgValue < TH1
                %blob_count = 0;
            %elseif sum(binary_map(:)) > binary_TH
               % blob_count = 2;
           % else
                %blob_count = 1;
            %end

            %identify people using max value
            row_1and2 = reshapedData(1:2,:);
            sorted = sort(row_1and2(:), "descend");
            top_8 = sorted(1:8);
            disp(mean(top_8));
            if ~isempty(row_1and2_old)
                if max(row_1and2(:)) < 35 && max(row_1and2_old(:)) > 35
                    if mean(top_8_old) > TH2 %identify value before going
                       disp(mean(top_8_old))
                       count = count + 2;
                   else 
                       count = count + 1;
                   end
                end
            end
            row_1and2_old = row_1and2;
            top_8_old = top_8;
            %disp(mean(top_8));
            
            %identify decrement
            last_rows = reshapedData(7:8,:);
            sorted_last = sort(last_rows(:), "descend");
            top_8_last = sorted_last(1:8);
            if ~isempty(last_rows_old)
                if max(last_rows(:)) < 35 && max(last_rows_old(:)) > 35
                   if mean(top_8_last_old) > TH2
                       disp(mean(top_8_old))
                       count = count - 2
                   else 
                       count = count - 1;
                   end
                end
            end
            last_rows_old = last_rows;
            top_8_last_old = top_8_last;


            %disp(['person crossing?',string(person_crossing)])
            disp(['old number of blobs: ', num2str(old_blob_count)])
            disp(['new number of blobs: ', num2str(blob_count)])
            
            
            %statements to increment/decrement
            
            if blob_count < old_blob_count %&& person_crossing == true
                count = count + old_blob_count - blob_count;
            end
            
            old_blob_count = blob_count;

            disp(['Average temperature value: ', num2str(avgValue)]);  % Display the average value
            disp(['Max temperature value: ', num2str(maxValue)]);
            % Remove the first 64 points from allData if you want to continue reading
            allData = allData(65:end);
            %disp(['mid_value = ', num2str(mid_value)])
            disp(['count = ', num2str(count)])
            end
            % creates image
            h.CData = reshapedData;
            %disp(mean(reshapedData(:)));
        end
    end
end















