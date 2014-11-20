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

#ifndef hAtomicData_H
#define hAtomicData_H

#include "hConstTypes.h"

class cAtomicData{
	private:
		sAtDa AtDa[NE];
		sfep fep[NE];
		void AtomicData();
		void AtomicParameterization();	
	public:
		void ReadAtomicData(int PotPar, int nAtomTypes, sAtomTypesCPU *AtomTypes, double Vrl=0);
};

#endif