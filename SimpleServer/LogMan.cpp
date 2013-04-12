#include "stdafx.h"

#include "LogMan.h"

/*static*/ LogMan::RedirectOutputFunc LogMan::sRedirectOutput = NULL;

/*static*/ void LogMan::SetRedirectOutputFunc(RedirectOutputFunc func)
{
	sRedirectOutput = func;
}

/*static*/ void LogMan::Log(const char* format, ...)
{
   char buffer[kMaxSize];

   va_list argptr;
   va_start(argptr, format);
   wvsprintf(buffer, format, argptr);
   va_end(argptr);

   if (sRedirectOutput)
   {
	   sRedirectOutput(buffer);
   }
   OutputDebugString(buffer);
}

