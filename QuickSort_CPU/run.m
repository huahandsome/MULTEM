clear all;
clc;
m = 2e+8;
z = rand(m, 1);
% Mex file single core
tic; QuickSort(z); toc;
% Matlab function mul-core
tic; sort(z); toc;