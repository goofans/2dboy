#pragma once

/*
 * when we have _CRTDBG_MAP_ALLOC defined stdlib.h must be included before
 * crtdbg.h.  this header ensures the includes happen in the correct order
 * this should only be necessary during debug builds where we use the
 * visual studio c runtime library (CRT) to debug memory allocation.
 * details here:
 * http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=99187
 */

#ifdef _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#endif
