clc; clear all;
mexGPU(1, 'MULTEMMat.cu', 'D:/MULTEM/General_CPU_GPU', 'hQuadrature.cpp', 'hAtomicdata.cpp'...
    , 'hPotential_CPU.cpp', 'hMT_General_CPU.cpp', 'hMT_Specimen_CPU.cpp', 'hMatlab2Cpp.cpp', 'hMT_Detector_CPU.cpp'...
    , 'hMT_General_GPU.cu', 'hMT_Potential_GPU.cu', 'hMT_AtomTypes_GPU.cu', 'hMT_MicroscopeEffects_GPU.cu'...
    , 'hMT_IncidentWave_GPU.cu', 'hMT_Detector_GPU.cu', 'hMT_STEM_GPU.cu', 'hMT_MulSli_GPU.cu');