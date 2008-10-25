/* LotU: Error handling functions.

   Copyright (C) 1995, Jamie Lokier.
   Written for Rebellion Developments, Ltd.

   Permission to use, copy, modify and distribute this file for any
   purpose by Rebellion Developments, Ltd. is hereby granted.  If you
   want to use this file outside the company, please let me know.
*/

#include "3dc.h"
#include "fail.h"
#include "dxlog.h"

void
fail (const char * format, ...)
{
  va_list ap;

  LOGDXSTR(format);
  va_start (ap, format);
  if (format != 0)
    vfprintf (stderr, format, ap);
  va_end (ap);

  exit (EXIT_FAILURE);
}
#if 0
void FAILHandleCompilerWarningMessage(void)
{
	int temp;

	temp = D3DRMMAP_PERSPCORRECT;
	temp = D3DRMMAP_WRAPU;
	temp = D3DRMMAP_WRAPV;
	temp = D3DRMGROUP_ALLGROUPS;
}	
#endif