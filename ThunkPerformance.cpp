// ThunkPerformance.cpp
// build with "CL /clr ThunkPerformance.cpp /link TunkPerformanceManagedParts.obj"

#define _WIN32_WINNT 0x400
#include <windows.h>
#include <iostream>
using namespace std;

//////////////////////
// target functions called in different measurements: 
// fManaged, fNativeLocal, fNativeFromDLL, and fManagedFromDLL

static long g_l = 0;

void __stdcall fManagedLocal();

__declspec(noinline)
void __stdcall fNativeLocal()
{
  ++g_l;
}

class NativeClassWithManagedCode
{
public:
  void f() ;
  virtual void vf();
};

class NativeClassWithNativeCode
{
public:
  void f() 
  {
    ++g_l;
  };
  virtual void vf()
  {
    ++g_l;
  }
};

#pragma comment(lib, "TestLib.lib")
extern "C" __declspec(dllimport) void __stdcall fNativeFromDLL();
extern "C" __declspec(dllimport) void __stdcall fManagedFromDLL();

class __declspec(dllimport) NativeClassFromDLLWithNativeCode
{
public:
  void f();
  virtual void vf();
};

class __declspec(dllimport) NativeClassFromDLLWithManagedCode
{
public:
  void f();
  virtual void vf();
};

////////////////////
// declarations of functions that perform the actual measurings
int numberOfCalls = 100000000;

typedef void (__stdcall* PFN)();

void __stdcall MeasureCallsFromNativeCaller
	(PFN pfn, const char* szFunctionName, bool bCallOnce);
void __stdcall MeasureCallsFromManagedCaller
	(PFN pfn, const char* szFunctionName, bool bCallOnce);

#define IMPLEMENT_NATIVE_MEASURE_FUNCTION(targetfn) \
void __stdcall Call_from_native_code_##targetfn() \
{ \
  for (int i = 0; i < numberOfCalls; ++i) \
    targetfn(); \
}

#define IMPLEMENT_NATIVE_MEASURE_FUNCTION_INDIRECT_CALL(targetfn) \
void __stdcall Call_indirectly_from_native_code_##targetfn() \
{ \
  PFN pfn = &targetfn; \
  for (int i = 0; i < numberOfCalls; ++i) \
    pfn(); \
}

IMPLEMENT_NATIVE_MEASURE_FUNCTION(fManagedLocal)
IMPLEMENT_NATIVE_MEASURE_FUNCTION(fNativeLocal)
IMPLEMENT_NATIVE_MEASURE_FUNCTION_INDIRECT_CALL(fManagedLocal)
IMPLEMENT_NATIVE_MEASURE_FUNCTION_INDIRECT_CALL(fNativeLocal)
IMPLEMENT_NATIVE_MEASURE_FUNCTION(fManagedFromDLL)
IMPLEMENT_NATIVE_MEASURE_FUNCTION(fNativeFromDLL)
IMPLEMENT_NATIVE_MEASURE_FUNCTION_INDIRECT_CALL(fManagedFromDLL)
IMPLEMENT_NATIVE_MEASURE_FUNCTION_INDIRECT_CALL(fNativeFromDLL)

#define DECLARE_MANAGED_MEASURE_FUNCTION(targetfn) \
void __stdcall Call_from_managed_code_##targetfn();

#define DECLARE_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(targetfn) \
void __stdcall Call_indirectly_from_managed_code_##targetfn();

DECLARE_MANAGED_MEASURE_FUNCTION(fManagedLocal)
DECLARE_MANAGED_MEASURE_FUNCTION(fNativeLocal)
DECLARE_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fManagedLocal)
void __stdcall Call_via_clrcall_pointer_from_managed_code_fManagedLocal();
DECLARE_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fNativeLocal)
DECLARE_MANAGED_MEASURE_FUNCTION(fManagedFromDLL)
DECLARE_MANAGED_MEASURE_FUNCTION(fNativeFromDLL)
DECLARE_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fManagedFromDLL)
DECLARE_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fNativeFromDLL)

#define IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(classname, memberfn) \
void __stdcall Call_member_from_native_code_##classname##memberfn() \
{ \
  classname* p = new classname(); \
  for (int i = 0; i < numberOfCalls; ++i) \
    p->memberfn(); \
  delete p; \
}

IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassWithManagedCode, f);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassWithNativeCode, f);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassWithManagedCode, vf);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassWithNativeCode, vf);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithManagedCode, f);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithNativeCode, f);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithManagedCode, vf);
IMPLEMENT_NATIVE_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithNativeCode, vf);

#define DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(classname, memberfn) \
	void __stdcall Call_member_from_managed_code_##classname##memberfn();

DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithManagedCode, f);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithNativeCode, f);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithManagedCode, vf);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithVirtualClrCallFunction, vf);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithNativeCode, vf);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithManagedCode, f);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithNativeCode, f);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithManagedCode, vf);
DECLARE_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithNativeCode, vf);

struct MeasureData
{
  const char* funcName;
  PFN testFunc;
  bool managedCaller;
  bool indirectCall;
};

#define MANAGEDCALLER = 1
#define NATIVECALLER = 2
#define DIRECTCALL = 1
#define INDIRECTCALL = 2
#define MEASURE_ENTRY_MANAGEDCALLER_DIRECTCALL(targetfn) \
  { #targetfn, Call_from_managed_code_##targetfn, true, false }
#define MEASURE_ENTRY_MANAGEDCALLER_INDIRECTCALL(targetfn) \
  { #targetfn, Call_indirectly_from_managed_code_##targetfn, true, true }
#define MEASURE_ENTRY_NATIVECALLER_DIRECTCALL(targetfn) \
  { #targetfn, Call_from_native_code_##targetfn, false, false }
#define MEASURE_ENTRY_NATIVECALLER_INDIRECTCALL(targetfn) \
  { #targetfn, Call_indirectly_from_native_code_##targetfn, false, true }
#define MEASURE_ENTRY_MANAGEDCALLER_MEMBER(classname, memberfn) \
  { #classname "::" #memberfn, \
    Call_member_from_managed_code_##classname##memberfn, true, false }
#define MEASURE_ENTRY_NATIVECALLER_MEMBER(classname, memberfn) \
  { #classname "::" #memberfn, \
    Call_member_from_native_code_##classname##memberfn, false, false }

void Measure(MeasureData* pMeasureData, int cMeasureData)
{
  for (int iEntry = 0; iEntry < cMeasureData; ++iEntry)
  {
    MeasureData& md = pMeasureData[iEntry];
    if (md.managedCaller)
      MeasureCallsFromManagedCaller(md.testFunc, md.funcName, md.indirectCall);
    else
      MeasureCallsFromNativeCaller(md.testFunc, md.funcName, md.indirectCall);
  }
}
///////////////////////
// Entry point
int main()
{
  // execute once to ensure JIT compilation has been done
  fManagedLocal(); fManagedFromDLL();
  NativeClassWithManagedCode n1; n1.f(); n1.vf();
  NativeClassFromDLLWithManagedCode n2; n2.f(); n2.vf();

  MeasureData md1[] =
  {
    MEASURE_ENTRY_MANAGEDCALLER_DIRECTCALL  (fManagedLocal  ),
    MEASURE_ENTRY_NATIVECALLER_DIRECTCALL   (fManagedLocal  ),
    MEASURE_ENTRY_MANAGEDCALLER_INDIRECTCALL(fManagedLocal  ),
	{"fManagedLocal (via __clrcall pointer)", 
	 Call_via_clrcall_pointer_from_managed_code_fManagedLocal,
	 true, false},
    MEASURE_ENTRY_NATIVECALLER_INDIRECTCALL (fManagedLocal  ),
    MEASURE_ENTRY_MANAGEDCALLER_DIRECTCALL  (fManagedFromDLL),
    MEASURE_ENTRY_NATIVECALLER_DIRECTCALL   (fManagedFromDLL),
    MEASURE_ENTRY_MANAGEDCALLER_INDIRECTCALL(fManagedFromDLL),
    MEASURE_ENTRY_NATIVECALLER_INDIRECTCALL (fManagedFromDLL),
    MEASURE_ENTRY_MANAGEDCALLER_DIRECTCALL  (fNativeLocal   ),
    MEASURE_ENTRY_NATIVECALLER_DIRECTCALL   (fNativeLocal   ),
    MEASURE_ENTRY_MANAGEDCALLER_INDIRECTCALL(fNativeLocal   ),
    MEASURE_ENTRY_NATIVECALLER_INDIRECTCALL (fNativeLocal   ),
    MEASURE_ENTRY_MANAGEDCALLER_DIRECTCALL  (fNativeFromDLL ),
    MEASURE_ENTRY_NATIVECALLER_DIRECTCALL   (fNativeFromDLL ),
    MEASURE_ENTRY_MANAGEDCALLER_INDIRECTCALL(fNativeFromDLL ),
    MEASURE_ENTRY_NATIVECALLER_INDIRECTCALL (fNativeFromDLL )
  };
  Measure(md1, sizeof(md1)/sizeof(*md1));

  MeasureData md2[] =
  {
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassWithManagedCode       , f ),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassWithManagedCode       , f ),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassWithManagedCode       , vf),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassWithVirtualClrCallFunction, vf),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassWithManagedCode       , vf),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassFromDLLWithManagedCode, f ),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassFromDLLWithManagedCode, f ),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassFromDLLWithManagedCode, vf),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassFromDLLWithManagedCode, vf),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassWithNativeCode        , f ),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassWithNativeCode        , f ),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassWithNativeCode        , vf),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassWithNativeCode        , vf),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassFromDLLWithNativeCode , f ),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassFromDLLWithNativeCode , f ),
    MEASURE_ENTRY_MANAGEDCALLER_MEMBER(NativeClassFromDLLWithNativeCode , vf),
    MEASURE_ENTRY_NATIVECALLER_MEMBER (NativeClassFromDLLWithNativeCode , vf)
  };
  Measure(md2, sizeof(md2)/sizeof(*md2));
}

///////////////////////
// helper class for performance measurements

class MeasureHelper
{
  static long long perfFreq;
  int threadPriority;
  long long startCounter, finishCounter;

public:
  void StartMeasuring()
  {
    if (perfFreq == 0)
      ::QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);

    // boost priority to avoid thread scheduling side effects
    threadPriority = ::GetThreadPriority(::GetCurrentThread());  
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    ::QueryPerformanceCounter((LARGE_INTEGER*)&startCounter);
  }

  void StopMeasuring()
  {
    ::QueryPerformanceCounter((LARGE_INTEGER*)&finishCounter);

    ::SetThreadPriority(::GetCurrentThread(), threadPriority); 
  }

  double GetMeasuredTime()
  {
    return (finishCounter - startCounter) / (double)perfFreq;
  }
};

long long MeasureHelper::perfFreq = 0;

void __stdcall MeasureCallsFromNativeCaller
	(PFN pfn, const char* szFunctionName, bool bIndirect)
{
  MeasureHelper mh;
  mh.StartMeasuring();
  pfn();
  mh.StopMeasuring();

  cout << mh.GetMeasuredTime() 
       << "s\tfor 10e8 " << (bIndirect ? "indirect " : "") << "calls to " 
       << szFunctionName << " from native code" << endl;
}
