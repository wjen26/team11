clear; clc; close all;
clear s;

% Setup the serial port
s = serialport('COM5', 9600);
configureTerminator(s, "LF");

allData = [];  % Initialize an empty array to store numeric data

% Create a figure window before the loop starts
figure;
h = imagesc(zeros(8, 8));  % Initialize with a blank 8x8 matrix
colormap('cool');  
colorbar;  % Show the colorbar

% Set the fixed color axis range between 15 and 35
%caxis([0, 40]);  % Keep the color scale fixed between 15 and 35 for temperature value

%initialize old blob count and new blob count
blob_count = 0;
old_blob_count = 0;
%create threshold temp

%initialize num_features
old_num_features = 0;

%initialize count
count = 0;

% Initialize calibration step
need_cal = true;
cal_count = 0;
Tamb = 0; calibration_matrices = {}; TH1 = 0; TH2 = 30.52; binary_TH = 17; ch = 15;

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
            if cal_count == 13
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
            
                if cal_count > 11
                    disp("step 3")
                    TH1 = cal_step_3(calibration_matrices);
                    
                    % Remove the first 64 points from allData if you want to continue reading
                    allData = allData(65:end);
                end
            end
            cal_count = cal_count + 1;
            
            % creates image
            h.CData = reshapedData;
            

            % Label connected components
            %[labeled_array, num_features] = bwlabel(binary_map);
            % Display results
            %disp(labeled_array);
            %disp(['Number of blobs ', num2str(num_features)]);
            
            %Person crossing boolian
            person_crossing = false;
            
            % Get the indices of each feature
            %for feature_idx = 1:num_features
                % Find the indices of the pixels corresponding to this feature
                %[row, col] = find(labeled_array == feature_idx);
    
                % Store the indices in the cell array
                %feature_indices{feature_idx} = [row, col];
            %end
            
            % Now to access the row indices for each feature:
            %for feature_idx = 1:num_features
                % Get the indices for this feature
                %indices = feature_indices{feature_idx};
    
                % Extract row indices (first column of the indices matrix)
                %rows = indices(:, 1);
                % Check if any row value is 0
                %if any(rows == 0)
                  %person_crossing = true;
                %end
            %end
            
            %disp(['person crossing?',string(person_crossing)
            
            
            if need_cal == false
            
            reshapedData = real_temp(reshapedData,ch,Tamb)
            matrices{end+1} = reshapedData;
            mid_value(end+1) = mean(mean(reshapedData(4:5,4:5)));
            
            % Calculate the average of the 8x8 matrix
            avgValue = mean(reshapedData(:));  % Compute the average of all values in the matrix
            maxValue = max(reshapedData(:));
            binary_map = reshapedData > TH2;
            weighted_binary = binary_map.*reshapedData;
            binary_vec(end+1) = sum(binary_map(:));
            
            if avgValue < TH1
                blob_count = 0;
            elseif sum(binary_map(:)) > binary_TH
                blob_count = 2;
            else
                blob_count = 1;
            end

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

        end
    end
end















