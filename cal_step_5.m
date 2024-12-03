function [TH2] = cal_step_5(calibration_matrices)
std_values = [];
mean_values = [];
    for i = 1:length(calibration_matrices)
        
        matrix = calibration_matrices{1,i};
        row_1and2 = matrix(1:2,:);
        sorted = sort(row_1and2(:), "descend");
        top_8 = sorted(1:8);
        mean_values(end+1) = mean(top_8);
        std_values(end+1) = std(top_8);
    end
    disp("std of means")
    disp(std(mean_values))
    TH2 = mean(mean_values) + 2.5*std(mean_values);
end
