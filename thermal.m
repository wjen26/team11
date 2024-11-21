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
caxis([0, 5]);  % Keep the color scale fixed between 15 and 35 for temperature values

%Create a change in average threshold
delta_T = 5;

%create threshold temp
thresh = 24;

%initialize num_features
old_num_features = 0;

%initialize count
count = 0;

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
            
            %optional Savitsky Golay filter, comment out if dont want
            %polynomialOrder = 3; frameSize = 5; reshapedData = sgolayfilt(reshapedData, polynomialOrder, frameSize);

            % top-hat filter
            se = strel('square', 2);
            reshapedData = imtophat(reshapedData, se);
            
            
            % Define standard deviation (sigma)
            sigma = 1;

            % Create Gaussian filter coefficients
            windowSize = 5; % Must be odd
            x = -floor(windowSize/2):floor(windowSize/2);
            gaussianKernel = exp(-x.^2 / (2 * sigma^2));
            gaussianKernel = gaussianKernel / sum(gaussianKernel); % Normalize

            % Apply the Gaussian filter
            %reshapedData = conv2(reshapedData, gaussianKernel, 'full');
            
            binary_map = reshapedData > thresh;
            disp(reshapedData);
            disp(binary_map)
            
            % creates image
            h.CData = reshapedData;
             
            % Label connected components
            [labeled_array, num_features] = bwlabel(binary_map);
            % Display results
            disp(labeled_array);
            %disp(['Number of blobs ', num2str(num_features)]);
            
            %Person crossing boolian
            person_crossing = false;
            
            % Get the indices of each feature
            for feature_idx = 1:num_features
                % Find the indices of the pixels corresponding to this feature
                [row, col] = find(labeled_array == feature_idx);
    
                % Store the indices in the cell array
                feature_indices{feature_idx} = [row, col];
            end
            
            % Now to access the row indices for each feature:
            for feature_idx = 1:num_features
                % Get the indices for this feature
                indices = feature_indices{feature_idx};
    
                % Extract row indices (first column of the indices matrix)
                rows = indices(:, 1);
                % Check if any row value is 0
                if any(rows == 0)
                  person_crossing = true;
                end
            end
            
            disp(['person crossing?',string(person_crossing)])
            disp(['old number of blobs: ', num2str(old_num_features)])
            disp(['new number of blobs: ', num2str(num_features)])
            %statements to increment/decrement
            if num_features < old_num_features %&& person_crossing == true
                count = count + old_num_features - num_features;
            end
            old_num_features=num_features;
            
            % Calculate the average of the 8x8 matrix
            avgValue = mean(reshapedData(:));  % Compute the average of all values in the matrix
            maxValue = max(reshapedData(:));
            disp(['Average temperature value: ', num2str(avgValue)]);  % Display the average value
            disp(['Max temperature value: ', num2str(maxValue)]);
            % Remove the first 64 points from allData if you want to continue reading
            allData = allData(65:end);
            disp(['count = ', num2str(count)])
            

            

        end
    end
end















