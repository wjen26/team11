function [TH1] = cal_step_3(calibration_matrices)
std_values = [];
mean_values = [];
    for i = 1:length(calibration_matrices)
        
        matrix = calibration_matrices{1,i};
        
        mean_values(end+1) = mean(matrix(:));
        std_values(end+1) = std(matrix(:));
    end
    disp("mean of standard devs")
    disp(std(mean_values))
    TH1 = mean(mean_values) + 2.5*std(mean_values);
end

