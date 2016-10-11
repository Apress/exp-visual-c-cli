// TestLib.cpp
// compile with "CL /c /clr TestLibPart2.cpp"

static long g_l = 0;

extern "C" __declspec(dllexport)
void __stdcall fManagedFromDLL() 
{
  ++g_l;
}

class __declspec(dllexport) NativeClassFromDLLWithManagedCode
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