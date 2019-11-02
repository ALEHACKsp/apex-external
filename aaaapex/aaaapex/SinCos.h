#pragma once

extern "C"
{

#if defined(_M_AMD64)

	// 64 bit implementation in MASM
	// VS2017> enable MASM by right clicking your project in solution explorer then
	// Build Dependencies > Build Customizations > MASM
	extern void SinCos(double AngleInRadians, double* pSinAns, double* pCosAns);
	// Add your assembly file as a source, but exclude it from 32 bit build

#else

	// 32 bit implementation in inline assembly
	inline void SinCos(double AngleInRadians, double* pSinAns, double* pCosAns)
	{
		double SinAns, CosAns;
		__asm {
			fld QWORD PTR[AngleInRadians]
			fsincos
			fstp QWORD PTR[CosAns]
			fstp QWORD PTR[SinAns]
			fwait
		}
		*pSinAns = SinAns;
		*pCosAns = CosAns;
	}

#endif

} // extern "C"