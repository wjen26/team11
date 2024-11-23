
%Get mean values distribution
std_values = [];
mean_values = [];
max_values = [];
for i = 8:25
    matrix = matrices{1,i};
    mean_values(end+1) = mean(matrix(:));
    std_values(end+1) = std(matrix(:));
    max_values(end+1) = max(matrix(:));
end
figure
stem(mean_values)
figure
stem(std_values)
disp(mean(mean_values));
disp(std(mean_values));
figure
stem(max_values)