// TestLib.cpp
// build with "CL /MD /LD TestLib.cpp /link TestLibManagedParts.obj"

static long g_l = 0;

extern "C" __declspec(dllexport)
void __stdcall fNativeFromDLL() 
{
  ++g_l;
}

class __declspec(dllexport) NativeClassFromDLLWithNativeCode
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