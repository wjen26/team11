function [Tv] = real_temp(Tsa,h,Tamb)
%REAL_TEMP Summary of this function goes here
%   Detailed explanation goes here
sigma = 5.67e-8;
eps = 0.98;
Tv = ((Tsa+273).^4 + h*(Tsa- Tamb)/(sigma*eps) ).^0.25 - 273;
end
