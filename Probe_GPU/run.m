clear all; clc;

Probe.gpu = 0;            % Gpu card
Probe.E0 = 300;
Probe.theta = 0.0; Probe.phi = 0; % Till ilumination (degrees)
Probe.lx = 10; Probe.ly = 10;
Probe.nx = 1024; Probe.ny = 1024;
Probe.x0 = 0.5*Probe.lx; Probe.y0 = 0.5*Probe.ly;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Probe.m = 0;        %mm
Probe.f = 88.7414;   %Angs
Probe.Cs3 = 0.04;	%mm
Probe.Cs5 = 0.00;	%mm
Probe.mfa2 = 0.0; Probe.afa2 = 0.0; %(Angs, degrees)
Probe.mfa3 = 0.0; Probe.afa3 = 0.0; %(Angs, degrees)
Probe.aobjl = 0.0; Probe.aobju = 21.0659; %(mrad, mrad)
Probe.sf = 32; Probe.nsf = 10; % (Angs, number of steps)ne
Probe.beta = 0.2; Probe.nbeta = 10; %(mrad, half number of steps)

tic;
Psi0 = getProbe(Probe);
toc;
fPsi0 = fftshift(fft2(ifftshift(Psi0)));
figure(1);
subplot(1, 2, 1);
imagesc(abs(Psi0));
colormap hot;
axis image;
subplot(1, 2, 2);
imagesc(angle(fPsi0));
colormap hot;
axis image;

