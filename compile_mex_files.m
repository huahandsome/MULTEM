clear all; clc;
addpath([ pwd '/Matlab_functions'])

files = {'mex_radial_distribution_2D', ...
        'mex_filter_Butterworth_2D', ...
        'mex_filter_Gaussian_2D', ...
        'mex_filter_Hanning_2D', ...
        'mex_FFT_information_limit_2D', ...
        'mex_fxeg_tabulated_data', ...
        'mex_quadrature', ...
        'mex_feg', ...
        'mex_fxg', ...
        'mex_Pr', ...
        'mex_Vz', ...
        'mex_Vr', ...
        'mex_Vp', ...
        'mex_crystal_by_layers', ...
        'mex_atom_radius', ...
        'mex_atom_type', ...
        'mex_specimen_slicing', ...
        'mex_projected_potential', ...
        'mex_transmission_function', ... 
        'mex_wave_function', ...
        'mex_probe', ... 
        'mex_propagate', ...          
        'mex_microscope_aberrations', ...        
        'mex_MULTEM'};
    
for file=files
  disp(['Compiling ' file{1}])
  run(['mex_files/', file{1}])
end;