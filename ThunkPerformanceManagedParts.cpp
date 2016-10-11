// ThunkPerformanceManagedParts.cpp
// build with "CL /c /clr ThunkPerformanceManagedParts.cpp"

#define _WIN32_WINNT 0x400
#include <windows.h>
#include <iostream>
using namespace std;

static long g_l = 0;

using namespace System::Runtime::CompilerServices;

//////////////////////
// target functions called in different measurements: 
// fManaged, fNativeLocal, fNativeFromDLL, and fManagedFromDLL

[MethodImpl(MethodImplOptions::NoInlining)]
__declspec(noinline)
void __stdcall fManagedLocal() 
{
  ++g_l;
}
extern void __stdcall fNativeLocal();

class NativeClassWithNativeCode
{
public:
  void __thiscall f();
  virtual void __thiscall vf();
};
class NativeClassWithManagedCode
{
public:
  void f()
  {
    ++g_l;
  }
  virtual void vf()
  {
    ++g_l;
  }
};
class NativeClassWithVirtualClrCallFunction
{
public:
  virtual void __clrcall vf()
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

///////////////////////
// helper class for performance measurements
class MeasureHelper
{
  static long long perfFreq;
  int threadPriority;
  long long startCounter, finishCounter;

public:
  void StartMeasuring();
  void StopMeasuring();
  double GetMeasuredTime();
};

///////////////////////
// functions that perform the actual measurings
extern int numberOfCalls;

typedef void (__stdcall* PFN)();

void __stdcall MeasureCallsFromManagedCaller
	(PFN pfn, const char* szFunctionName, bool bIndirect)
{
  MeasureHelper mh;
  mh.StartMeasuring();
  pfn();
  mh.StopMeasuring();

  cout << mh.GetMeasuredTime() 
       << "s\tfor 10e8 " << (bIndirect ? "indirect " : "") << "calls to " 
       << szFunctionName << " from managed code" << endl;
}

#define IMPLEMENT_MANAGED_MEASURE_FUNCTION(targetfn) \
void __stdcall Call_from_managed_code_##targetfn() \
{ \
  for (int i = 0; i < numberOfCalls; ++i) \
    targetfn(); \
}

#define IMPLEMENT_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(targetfn) \
void __stdcall Call_indirectly_from_managed_code_##targetfn() \
{ \
  PFN pfn = &targetfn; \
  for (int i = 0; i < numberOfCalls; ++i) \
    pfn(); \
}

IMPLEMENT_MANAGED_MEASURE_FUNCTION(fManagedLocal)
IMPLEMENT_MANAGED_MEASURE_FUNCTION(fNativeLocal)
IMPLEMENT_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fManagedLocal)
IMPLEMENT_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fNativeLocal)
IMPLEMENT_MANAGED_MEASURE_FUNCTION(fManagedFromDLL)
IMPLEMENT_MANAGED_MEASURE_FUNCTION(fNativeFromDLL)
IMPLEMENT_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fManagedFromDLL)
IMPLEMENT_MANAGED_MEASURE_FUNCTION_INDIRECT_CALL(fNativeFromDLL)

void __stdcall Call_via_clrcall_pointer_from_managed_code_fManagedLocal()
{
  void (__clrcall* pfn)() = &fManagedLocal;
  for (int i = 0; i < numberOfCalls; ++i)
    pfn();
}


#define IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(classname, targetfn) \
void __stdcall Call_member_from_managed_code_##classname##targetfn() \
{ \
  classname* p = new classname(); \
  for (int i = 0; i < numberOfCalls; ++i) \
    p->targetfn(); \
  delete p; \
}

IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithManagedCode, f);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithNativeCode, f);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithManagedCode, vf);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithVirtualClrCallFunction,vf);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassWithNativeCode, vf);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithManagedCode, f);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithNativeCode, f);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithManagedCode, vf);
IMPLEMENT_MANAGED_MEASURE_MEMBERFUNCTION(NativeClassFromDLLWithNativeCode, vf);
