clc; clear all;
mexGPU(1, 'PCBFTEM.cu', 'D:/MULTEM/General_CPU_GPU', 'hQuadrature.cpp', 'hAtomicdata.cpp'...
    , 'hPotential_CPU.cpp', 'hMT_General_CPU.cpp', 'hMT_Specimen_CPU.cpp', 'hMatlab2Cpp.cpp'...
    , 'hMT_General_GPU.cu', 'hMT_MicroscopeEffects_GPU.cu', 'hTEMIm.cu');