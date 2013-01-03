
#ifndef _BOY_MATH_H_
#define _BOY_MATH_H_

#if defined(GOO_PLATFORM_WII)

inline float boy_sqrtf( register const float val )
{
	register float rec_estimate;
	register float estimate;
	
	if( val == 0.0f ) return 0.0f;

	asm
	{
		frsqrte		rec_estimate, val
		fres		estimate, rec_estimate

		// this will take care of the commented out part of the first line of C code below
		fmadds		estimate, val, rec_estimate, estimate
	}

	estimate = /*(estimate + (val / estimate))*/ estimate * 0.5f;
	estimate = (estimate + (val / estimate)) * 0.5f;
	return estimate;
}

#else

#ifdef __cplusplus

inline float boy_sqrtf( register const float val )
{
	return sqrtf( val );
}

#else

#endif

#endif

#endif
