// TrustStrongNameFromAssembly.cpp
// build with "cl /clr TrustStrongNameFromAssembly.cpp"

#include <vcclr.h>
#include <strongname.h>

#using <System.dll>

using namespace System;
using namespace System::Collections;
using namespace System::Text;
using namespace System::Runtime::InteropServices;

using namespace System::Security;
using namespace System::Security::Policy;
using namespace System::Security::Permissions;

void DumpHelp()
{
  Console::WriteLine("Usage: TrustStrongName <assembly signed with public key to trust>");
}

PolicyLevel^ FindPolicyLevel(String^ name)
{
  IEnumerator^ levels = SecurityManager::PolicyHierarchy();
  while (levels->MoveNext())
  {
    PolicyLevel^ level = safe_cast<PolicyLevel^>(levels->Current);
    if (level->Label == name)
      return level;
  }
  return nullptr;
}

NamedPermissionSet^ FindNamedPermissionSet(PolicyLevel^ policyLevel, String^ name)
{
  if (!policyLevel)
    throw gcnew ArgumentNullException("policyLevel");
  if (!name)
    throw gcnew ArgumentNullException("name");

  for each(NamedPermissionSet^ nps in policyLevel->NamedPermissionSets)
    if (nps->Name == name)
      return nps;

  return nullptr;
}

CodeGroup^ FindCodeGroup(CodeGroup^ parent, String^ name)
{
  if (!parent)
    throw gcnew ArgumentNullException("parent");
  if (!name)
    throw gcnew ArgumentNullException("name);

  for each (CodeGroup^ cg in parent->Children)
    if (cg->Name == name)
      return cg;

  return nullptr;
}

void GetPublicKeyAndPublicKeyToken(
		String^ assemblyFilename, 
		[Out] array<Byte>^% publicKey, 
		[Out] array<Byte>^% publicKeyToken)
{
  pin_ptr<wchar_t const> wszAssemblyFileName = ::PtrToStringChars(assemblyFilename);
  BYTE      *pbStrongNameToken = 0;
  ULONG     cbStrongNameToken = 0;
  BYTE      *pbPublicKeyBlob = 0;
  ULONG     cbPublicKeyBlob = 0;
  if (!StrongNameTokenFromAssemblyEx(wszAssemblyFileName, 
                                     &pbStrongNameToken, 
                                     &cbStrongNameToken,
                                     &pbPublicKeyBlob,
                                     &cbPublicKeyBlob))
  {
    DWORD err = StrongNameErrorInfo();
    throw gcnew Exception(
          String::Format("StrongNameTokenFromAssemblyEx caused error {0}", err));
  }

  // this code copies
  publicKeyToken = gcnew array<Byte>(cbStrongNameToken);
  Marshal::Copy(IntPtr(pbStrongNameToken), publicKeyToken, 0, cbStrongNameToken);
  StrongNameFreeBuffer(pbStrongNameToken);

  publicKey = gcnew array<Byte>(cbPublicKeyBlob);
  Marshal::Copy(IntPtr(pbPublicKeyBlob), publicKey, 0, cbPublicKeyBlob);
  StrongNameFreeBuffer(pbPublicKeyBlob);
}

int main(array<System::String ^> ^args)
{
  array<Char>^ hexDigits = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

  Console::WriteLine("TrustStrongName");
  Console::WriteLine("Written by Marcus Heege - no warranties whatsoever");
  Console::WriteLine();

  if (args->Length != 1)
  {
    DumpHelp();
    return 1;
  }
  if (args[0] == "/?" || args[0] == "-?")
  {
    DumpHelp();
    return 0;
  }

  try
  {
    array<Byte>^ publicKey = nullptr;
    array<Byte>^ publicKeyToken = nullptr;
    GetPublicKeyAndPublicKeyToken(args[0], publicKey, publicKeyToken);

    Console::WriteLine("Public key:");
    for each(Byte b in publicKey)
      Console::Write("{0}{1}", hexDigits[b >> 4], hexDigits[b & 0xF]);
    Console::WriteLine();

    StringBuilder sb(publicKeyToken->Length * 2);
    for each(Byte b in publicKeyToken)
      sb.AppendFormat("{0}{1}", hexDigits[b >> 4], hexDigits[b & 0xF]);
    String^ strPublicKeyToken = sb.ToString();
    Console::WriteLine("Public key token: {0}", strPublicKeyToken);

    PolicyLevel^ machineLevel = FindPolicyLevel("machine");
    if (!machineLevel)
      throw gcnew Exception("Machine level not found.");

    NamedPermissionSet^ npsFullTrust = FindNamedPermissionSet(machineLevel, "FullTrust");
    if (!npsFullTrust)
      throw gcnew Exception("FullTrust permission set not found");
    PolicyStatement^ polStmtFullTrust = gcnew PolicyStatement(npsFullTrust);

    String^ codeGroupName = String::Format("PKT{0}FullTrust", strPublicKeyToken);
    CodeGroup^ cgPKTFullTrust = FindCodeGroup(machineLevel->RootCodeGroup, 
                                              codeGroupName);
    if (cgPKTFullTrust)
    {
      Console::WriteLine("Deleting existing code group " + codeGroupName);
      machineLevel->RootCodeGroup->RemoveChild(cgPKTFullTrust);
    }

    Console::WriteLine("Adding new code group " + codeGroupName);

    StrongNamePublicKeyBlob^ snpkb = gcnew StrongNamePublicKeyBlob(publicKey);
    StrongNameMembershipCondition^ snmc = 
	gcnew StrongNameMembershipCondition(snpkb, nullptr, nullptr);
    cgPKTFullTrust = gcnew UnionCodeGroup(snmc, polStmtFullTrust);
    cgPKTFullTrust->Name = codeGroupName;
    machineLevel->RootCodeGroup->AddChild(cgPKTFullTrust);

    Console::WriteLine("Saving machine policy level");
    SecurityManager::SavePolicyLevel(machineLevel);
  }
  catch (Exception^ ex)
  {
    Console::WriteLine("Error occured: " + ex->Message);
    return 1;
  }

  Console::WriteLine("Succeeded");
  return 0;
}
