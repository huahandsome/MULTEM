/**
 *  This file is part of MULTEM.
 *  Copyright 2014 Ivan Lobato <Ivanlh20@gmail.com>
 *
 *  MULTEM is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  MULTEM is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with MULTEM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstring>
#include "hmathCPU.h"
#include "hConstTypes.h"
#include "hMT_General_CPU.h"
#include "hAtomicData.h"

// Input: E0(keV), Output: lambda (electron wave)
double f_getLambda(double E0){
	double emass = 510.99906;		// electron rest mass in keV
	double hc = 12.3984244;			// Planck's const x speed of light
		
	double lambda = hc/sqrt(E0*(2.0*emass + E0));
	return lambda;
}

// Input: E0(keV), Output: sigma (Interaction parameter)
double f_getSigma(double E0){
	double emass = 510.99906;		// electron rest mass in keV
	double hc = 12.3984244;			// Planck's const x speed of light
	double x = (emass + E0)/(2.0*emass + E0);
	
	double lambda = hc/sqrt(E0*(2.0*emass + E0));
	double sigma = 2.0*cPi*x/(lambda*E0);
	return sigma;
}

// Input: E0(keV), Output: gamma(relativistic factor)
double f_getGamma(double E0){
	double emass = 510.99906;		// electron rest mass in keV

	double gamma = (1.0 + E0/emass);
	return gamma;
}

// get index (with typ=0: bottom index for equal values and typ=1: upper index for equal values)
int f_getIndex(int ixmin, int ixmax, double *x, int typ, double x0){
	int ixmid;	
	switch (typ){
		case 0:
			do{
				ixmid = (ixmin + ixmax)>>1;		// divide by 2
				if(x0 <= x[ixmid]) 
					ixmax = ixmid;
				else 
					ixmin = ixmid;
			}while ((ixmax-ixmin)>1);
		case 1:
			do{
				ixmid = (ixmin + ixmax)>>1;		// divide by 2
				if(x0 < x[ixmid]) 
					ixmax = ixmid;
				else 
					ixmin = ixmid;
			}while ((ixmax-ixmin)>1);
	}

	if(x0==x[ixmax])
		return ixmax;
	else
		return ixmin;
}

// get two dimensional radial distribution for regular grid
void f_get2DRadDist(int nR, double *R, double *fR, int nRl, double *Rl, double *rl, double *frl, double *cfrl, bool reg, int typ){	
	int i, j;
	double Rlmin = Rl[0], Rlmax = Rl[nRl-1], dRl = Rl[1]-Rl[0];
 
	for (i=0; i<nRl-1; i++){
		rl[i] = 0.5*(Rl[i]+Rl[i+1]);
		frl[i] = 0.0;
		cfrl[i] = 0.0;
	}

	for (i=0; i<nR; i++)
		if ((Rlmin<=R[i])&&(R[i]<Rlmax)){
			j = (reg)?(int)floor((R[i]-Rlmin)/dRl):f_getIndex(0, nRl-1, Rl, 0, R[i]);
			frl[j] += fR[i];
			cfrl[j] += 1.0;
		}

	if (typ==0)
		for (i=0; i<nRl-1; i++)
			if (cfrl[i]>0)
				frl[i] /= cfrl[i];
}

// Set atom types
void f_SetAtomTypes(int Z, int PotPar, int ns, double Vrl, sAtomTypesCPU &AtomTypes){
	int nAtomTypes = 1;

	AtomTypes.Z = Z;
	AtomTypes.ns = ns;

	cAtomicData AtomicData;
	AtomicData.ReadAtomicData(PotPar, nAtomTypes, &AtomTypes, Vrl);
}

// Set atom types
void f_SetAtomTypes(int PotPar, int ns, double Vrl, int nAtomTypes, sAtomTypesCPU *&AtomTypes){
	for (int iAtomTypes=0; iAtomTypes<nAtomTypes; iAtomTypes++){
		AtomTypes[iAtomTypes].Z = iAtomTypes+1;
		AtomTypes[iAtomTypes].ns = ns;
		AtomTypes[iAtomTypes].Vo = 0; //Implementation in the future
	}
	cAtomicData AtomicData;
	AtomicData.ReadAtomicData(PotPar, nAtomTypes, AtomTypes, Vrl);
}

// Set Atoms
void f_AtomsM2Atoms(int nAtomsM_i, double *AtomsM_i, bool PBC_xyi, double lxi, double lyi, int &nAtoms, sAtoms *&Atoms){
	double x, y, dl = 1e-03;
	double lxb = lxi-dl, lyb = lyi-dl;

	Atoms = new sAtoms[nAtomsM_i];
	nAtoms = 0;
	for (int i=0; i<nAtomsM_i; i++){		
		x = AtomsM_i[0*nAtomsM_i + i];				// x-position
		y = AtomsM_i[1*nAtomsM_i + i];				// y-position
		if(!PBC_xyi||((x<lxb)&&(y<lyb))){
			Atoms[nAtoms].x = x;					// x-position
			Atoms[nAtoms].y = y;					// y-position
			Atoms[nAtoms].z = AtomsM_i[2*nAtomsM_i + i];		// z-position
			Atoms[nAtoms].Z = (int)AtomsM_i[3*nAtomsM_i + i];		// Atomic number
			Atoms[nAtoms].sigma = AtomsM_i[4*nAtomsM_i + i];		// Standard deviation
			Atoms[nAtoms].occ = AtomsM_i[5*nAtomsM_i + i];			// Occupancy
			nAtoms++;
		}
	}
}

// get 2D maximum interaction distance
double f_getRMax(int nAtoms, sAtoms *&Atoms, sAtomTypesCPU *&AtomTypes){
	double R, Rmax=0;

	for(int iAtoms=0; iAtoms<nAtoms; iAtoms++){
		R = AtomTypes[Atoms[iAtoms].Z-1].Rmax;
		if (Rmax < R)
			Rmax = R;
	}
	return Rmax;
}

/****************************************************************************/
/****************************************************************************/

// Grid's parameter initialization
void f_sGP_Init(sGP &GP){
	GP.nx = 0;
	GP.ny = 0;
	GP.nxh = 0;
	GP.nyh = 0;
	GP.nxy = 0;
	GP.inxy = 0;
	GP.lx = 0;
	GP.ly = 0;
	GP.dz = 0;
	GP.PBC_xy = true;
	GP.BWL = true;
	GP.dRx = 0;
	GP.dRy = 0;
	GP.dRmin = 0;
	GP.dgx = 0;
	GP.dgy = 0;
	GP.dgmin = 0;
	GP.gmax = 0;
	GP.gmax2 = 0;
	GP.gmaxl = 0;
	GP.gmaxl2 = 0;
}

// Grid's parameter calculation
void f_sGP_Cal(int nx, int ny, double lx, double ly, double dz, bool PBC_xy, bool BWL, sGP &GP){
	GP.nx = nx;
	GP.ny = ny;
	GP.nxh = nx/2;
	GP.nyh = ny/2;
	GP.nxy = GP.nx*GP.ny; 
	GP.inxy = (GP.nxy==0)?(0.0):(1.0/double(GP.nxy));
	GP.lx = lx;
	GP.ly = ly;
	GP.dz = dz;
	GP.PBC_xy = PBC_xy;
	GP.BWL = BWL;
	GP.dRx = (GP.nx==0)?(0.0):(GP.lx/double(GP.nx));
	GP.dRy = (GP.ny==0)?(0.0):(GP.ly/double(GP.ny));
	GP.dRmin = MIN(GP.dRx, GP.dRy);
	GP.dgx = (lx==0)?(0.0):(1.0/GP.lx);
	GP.dgy = (ly==0)?(0.0):(1.0/GP.ly);
	GP.dgmin = MIN(GP.dgx, GP.dgy);
	GP.gmax = MIN(double(GP.nxh)*GP.dgx, double(GP.nyh)*GP.dgy);
	GP.gmax2 = pow(GP.gmax, 2);
	GP.gmaxl = 2.0*GP.gmax/3.0;
	GP.gmaxl2 = pow(GP.gmaxl, 2);
}

/****************************************************************************/
/****************************************************************************/

void f_get_BTnxny(sGP &GP, dim3 &B, dim3 &T){
	B.x = (GP.ny+thrnxny-1)/thrnxny; B.y = (GP.nx+thrnxny-1)/thrnxny; B.z = 1;
	T.x = thrnxny; T.y = thrnxny; T.z = 1;
}

void f_get_BTnxhnyh(sGP &GP, dim3 &B, dim3 &T){
	B.x = (GP.nyh+thrnxny-1)/thrnxny; B.y = (GP.nxh+thrnxny-1)/thrnxny; B.z = 1;
	T.x = thrnxny; T.y = thrnxny; T.z = 1;
}

void f_get_BTmnxny(sGP &GP, dim3 &B, dim3 &T){
	B.x = (MAX(GP.nx, GP.ny)+thrnxy-1)/thrnxy; B.y = 1; B.z = 1;
	T.x = thrnxy; T.y = 1; T.z = 1;
}

void f_get_BTnxy(sGP &GP, dim3 &B, dim3 &T){
	B.x = MIN(64, (GP.nxy+thrnxy-1)/thrnxy); B.y = 1; B.z = 1;
	T.x = thrnxy; T.y = 1; T.z = 1;
}

/****************************************************************************/
/****************************************************************************/

// Block and Thread parameter initialization
void f_sLens_Init(sLens &Lens){
	Lens.gamma = 0;
	Lens.lambda = 0;
	Lens.m = 0;
	Lens.f = 0;
	Lens.Cs3 = 0;
	Lens.Cs5 = 0;
	Lens.mfa2 = 0;
	Lens.afa2 = 0;
	Lens.mfa3 = 0;
	Lens.afa3 = 0;
	Lens.aobjl = 0;
	Lens.aobju = 0;
	Lens.sf = 0;
	Lens.nsf = 0;
	Lens.beta = 0;
	Lens.nbeta = 0;
	Lens.lambda2 = 0;
	Lens.cf = 0;
	Lens.cCs3 = 0;
	Lens.cCs5 = 0;
	Lens.cmfa2 = 0;
	Lens.cmfa3 = 0;
	Lens.gmin2 = 0;
	Lens.gmax2 = 0;
	Lens.sggs = 0;
	Lens.ngxs = 0;
	Lens.ngys = 0;
	Lens.dgxs = 0;
	Lens.dgys = 0;
	Lens.gmax2s = 0;
}

// Lens' parameter calculation
void f_sLens_Cal(double E0, sGP &GP, sLens &Lens){
	Lens.gamma = f_getGamma(E0);

	Lens.lambda = f_getLambda(E0);
	Lens.lambda2 = pow(Lens.lambda, 2);

	Lens.cf = (Lens.f==0)?0:cPi*Lens.f*Lens.lambda;
	Lens.cCs3 = (Lens.Cs3==0)?0:-cPi*Lens.Cs3*pow(Lens.lambda, 3)/2.0;
	Lens.cCs5 = (Lens.Cs5==0)?0:-cPi*Lens.Cs5*pow(Lens.lambda, 5)/3.0;
	Lens.cmfa2 = (Lens.mfa2==0)?0:-cPi*Lens.mfa2*Lens.lambda;
	Lens.cmfa3 = (Lens.mfa3==0)?0:-2.0*cPi*Lens.mfa3*pow(Lens.lambda, 2)/3.0;
	Lens.gmin2 = (Lens.aobjl==0)?0:pow(Lens.aobjl/Lens.lambda, 2);
	Lens.gmax2 = (Lens.aobju==0)?0:pow(Lens.aobju/Lens.lambda, 2);

	double g0s = Lens.beta/Lens.lambda;
	Lens.sggs = g0s/c2i2;
	double gmaxs = 3.5*Lens.sggs;
	Lens.gmax2s = gmaxs*gmaxs;
	double dgs = gmaxs/Lens.nbeta;
	int nbeta;

	nbeta = (dgs<GP.dgx)?((int)floor(GP.dgx/dgs)+1):(1);
	Lens.ngxs = (int)floor(nbeta*gmaxs/GP.dgx) + 1;
	Lens.dgxs = gmaxs/Lens.ngxs;

	nbeta = (dgs<GP.dgy)?((int)floor(GP.dgy/dgs)+1):(1);
	Lens.ngys = (int)floor(nbeta*gmaxs/GP.dgy) + 1;
	Lens.dgys = gmaxs/Lens.ngys;
};

/****************************************************************************/
/****************************************************************************/

void f_sCoefPar_Free(sCoefPar &CoefPar){
	delete [] CoefPar.cl; CoefPar.cl = 0;
	delete [] CoefPar.cnl; CoefPar.cnl = 0;
}

void f_sCoefPar_Init(sCoefPar &CoefPar){
	CoefPar.cl = 0;
	CoefPar.cnl = 0;
}

void f_sCoefPar_Malloc(int nCoefPar, sCoefPar &CoefPar){
	if(nCoefPar<=0) return;

	CoefPar.cl = new double[nCoefPar];
	CoefPar.cnl = new double[nCoefPar];
}

/****************************************************************************/
/****************************************************************************/

void f_sciVn_Free(sciVn &ciVn){
	delete [] ciVn.c0; ciVn.c0 = 0;
	delete [] ciVn.c1; ciVn.c1 = 0;
	delete [] ciVn.c2; ciVn.c2 = 0;
	delete [] ciVn.c3; ciVn.c3 = 0;
}

void f_sciVn_Init(sciVn &ciVn){
	ciVn.c0 = 0;
	ciVn.c1 = 0;
	ciVn.c2 = 0;
	ciVn.c3 = 0;
}

void f_sciVn_Malloc(int nciVn, sciVn &ciVn){
	if(nciVn<=0) return;

	ciVn.c0 = new double[nciVn];
	ciVn.c1 = new double[nciVn];
	ciVn.c2 = new double[nciVn];
	ciVn.c3 = new double[nciVn];
}

/****************************************************************************/
/****************************************************************************/

void f_sDetCir_Free(sDetCir &DetCir){
	delete [] DetCir.g2min; DetCir.g2min = 0;
	delete [] DetCir.g2max; DetCir.g2max = 0;
}

void f_sDetCir_Init(sDetCir &DetCir){
	DetCir.g2min = 0;
	DetCir.g2max = 0;
}

void f_sDetCir_Malloc(int nDetCir, sDetCir &DetCir){
	if(nDetCir<=0) return;

	DetCir.g2min = new double[nDetCir];
	DetCir.g2max = new double[nDetCir];
}

/****************************************************************************/
/****************************************************************************/

void f_BuildGrid(int line, int ns, double x0, double y0, double xe, double ye, int &nxs, int &nys, double *&xs, double *&ys){
	if(ns<=0){
		nxs = nys = 0;
		xs = 0; ys = 0;
		return;
	}

	int i;
	double lxs = xe-x0, lys = ye-y0, ld = sqrt(lxs*lxs+lys*lys);
	double theta = atan(lys/lxs), costheta = cos(theta), sintheta = sin(theta);

	if (line){
			nxs = ns;
			nys = ns;
	}else{
		nxs = (abs(lxs)>abs(lys))?ns:(int)ceil(ns*abs(lxs/lys));
		nys = (abs(lxs)>abs(lys))?(int)ceil(ns*abs(lys/lxs)):ns;
	}

	xs = new double [nxs];
	ys = new double [nys];

	double ds;

	if (line){
		ds = ld/ns;
		for (i=0; i<ns; i++){
			xs[i] = x0 + i*ds*costheta;
			ys[i] = y0 + i*ds*sintheta;
		}
	}else{
		ds = lxs/nxs;
		for (i=0; i<nxs; i++)
			xs[i] = x0 + i*ds;

		ds = lys/nys;
		for (i=0; i<nys; i++)
			ys[i] = y0 + i*ds;
	}

}

/****************************************************************************/
/****************************************************************************/

void f_InMulSli_Init(sInMSTEM &InMSTEM){
	InMSTEM.gpu = 0;
	InMSTEM.SimType = 0;
	InMSTEM.nConfFP = 0;
	InMSTEM.DimFP = 0;
	InMSTEM.SeedFP = 0;
	InMSTEM.PotPar = 0;
	InMSTEM.MEffect = 0;		
	InMSTEM.STEffect = 0;	
	InMSTEM.ZeroDefTyp = 0;
	InMSTEM.ZeroDefPlane = 0;
	InMSTEM.ApproxModel = 0;
	InMSTEM.BandwidthLimit = 0;
	InMSTEM.ThicknessTyp = 0;
	InMSTEM.nThickness = 0;
	InMSTEM.Thickness = 0;

	InMSTEM.E0 = 0;
	InMSTEM.theta = 0;
	InMSTEM.phi = 0;
	InMSTEM.nx = 0;
	InMSTEM.ny = 0;
	InMSTEM.lx = 0;
	InMSTEM.ly = 0;
	InMSTEM.dz = 0;

	InMSTEM.MC_m = 0;
	InMSTEM.MC_f = 0;
	InMSTEM.MC_Cs3 = 0;
	InMSTEM.MC_Cs5 = 0;
	InMSTEM.MC_mfa2 = 0;
	InMSTEM.MC_afa2 = 0;
	InMSTEM.MC_mfa3 = 0;
	InMSTEM.MC_afa3 = 0;
	InMSTEM.MC_aobjl = 0;
	InMSTEM.MC_aobju = 0;
	InMSTEM.MC_sf = 0;
	InMSTEM.MC_nsf = 0;
	InMSTEM.MC_beta = 0;
	InMSTEM.MC_nbeta = 0;

	InMSTEM.nAtomsM = 0;
	InMSTEM.AtomsM = 0;

	InMSTEM.STEM_line = 0;
	InMSTEM.STEM_FastCal = false;
	InMSTEM.STEM_ns = 0;
	InMSTEM.STEM_x1u = 0;
	InMSTEM.STEM_y1u = 0;
	InMSTEM.STEM_x2u = 0;
	InMSTEM.STEM_y2u = 0;
	
	InMSTEM.STEM_nDet = 0;
	InMSTEM.STEM_DetCir = 0;

	InMSTEM.CBED_x0 = 0;
	InMSTEM.CBED_y0 = 0;
	InMSTEM.CBED_Space = 0;

	//InMSTEM.HRTEM_xx;

	InMSTEM.PED_nrot = 0;
	InMSTEM.PED_theta = 0;

	InMSTEM.HCI_nrot = 0;
	InMSTEM.HCI_theta = 0;
	//InMSTEM.HCI_xx;

	//InMSTEM.EWRS_xx;

	//InMSTEM.EWFS_xx;
}

void f_InMulSli_Free(sInMSTEM &InMSTEM){
	InMSTEM.gpu = 0;
	InMSTEM.SimType = 0;
	InMSTEM.nConfFP = 0;
	InMSTEM.DimFP = 0;
	InMSTEM.SeedFP = 0;
	InMSTEM.PotPar = 0;
	InMSTEM.MEffect = 0;		
	InMSTEM.STEffect = 0;	
	InMSTEM.ZeroDefTyp = 0;
	InMSTEM.ZeroDefPlane = 0;
	InMSTEM.ApproxModel = 0;
	InMSTEM.BandwidthLimit = 0;
	InMSTEM.ThicknessTyp = 0;
	InMSTEM.nThickness = 0;
	delete [] InMSTEM.Thickness; InMSTEM.Thickness = 0;

	InMSTEM.E0 = 0;
	InMSTEM.theta = 0;
	InMSTEM.phi = 0;
	InMSTEM.nx = 0;
	InMSTEM.ny = 0;
	InMSTEM.lx = 0;
	InMSTEM.ly = 0;
	InMSTEM.dz = 0;

	InMSTEM.MC_m = 0;
	InMSTEM.MC_f = 0;
	InMSTEM.MC_Cs3 = 0;
	InMSTEM.MC_Cs5 = 0;
	InMSTEM.MC_mfa2 = 0;
	InMSTEM.MC_afa2 = 0;
	InMSTEM.MC_mfa3 = 0;
	InMSTEM.MC_afa3 = 0;
	InMSTEM.MC_aobjl = 0;
	InMSTEM.MC_aobju = 0;
	InMSTEM.MC_sf = 0;
	InMSTEM.MC_nsf = 0;
	InMSTEM.MC_beta = 0;
	InMSTEM.MC_nbeta = 0;

	InMSTEM.nAtomsM = 0;
	InMSTEM.AtomsM = 0;

	InMSTEM.STEM_line = 0;
	InMSTEM.STEM_FastCal = false;
	InMSTEM.STEM_ns = 0;
	InMSTEM.STEM_x1u = 0;
	InMSTEM.STEM_y1u = 0;
	InMSTEM.STEM_x2u = 0;
	InMSTEM.STEM_y2u = 0;
	
	InMSTEM.STEM_nDet = 0;
	delete [] InMSTEM.STEM_DetCir; InMSTEM.STEM_DetCir = 0;

	InMSTEM.CBED_x0 = 0;
	InMSTEM.CBED_y0 = 0;
	InMSTEM.CBED_Space = 0;

	//InMSTEM.HRTEM_xx;

	InMSTEM.PED_nrot = 0;
	InMSTEM.PED_theta = 0;

	InMSTEM.HCI_nrot = 0;
	InMSTEM.HCI_theta = 0;
	//InMSTEM.HCI_xx;

	//InMSTEM.EWRS_xx;

	//InMSTEM.EWFS_xx;
}