using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Generic;

namespace LedgerKeePass
{
  using LedgerDevicePtr = IntPtr;
  using KPLPtr = IntPtr;

  internal static class Constants {
    public const int SlotCount = 8;
    public const int ResultSuccess = 0;
    public const int KeySize = 32;
    public const int MaxNameSize = 5;
#if OS_UNIX
    public const string KPLDllName = "libkpl_c.so";
#elif OS_WIN
    public const string KPLDllName = "kpl_c.dll";
#elif OS_OSX
    public const string KPLDllName = "kpl_c.dylib";
#endif
  }

  [Serializable]
  public class KPLException : Exception
  {
    public readonly int Res;

    private KPLException(int Res, string Msg):
      base(Msg)
    {
      this.Res = Res;
    }

    public static void process(int Res) {
      if (Res != Constants.ResultSuccess) {
        string Msg = Marshal.PtrToStringAnsi(CKPL.KPLErrorStr(Res));
        throw new KPLException(Res, Msg);
      }
    }
  }

  public class LedgerDevice {
    readonly public LedgerDevicePtr Dev;

    private LedgerDevice(LedgerDevicePtr Dev)
    {
      this.Dev = Dev;
    }

    ~LedgerDevice()
    {
      CKPL.KPLLedgerDeviceFree(this.Dev);
    }

    public static List<LedgerDevice> listDevices()
    {
      CKPL.LedgerDeviceList List = new CKPL.LedgerDeviceList();
      int Res = CKPL.KPLLedgerDeviceListGet(ref List);
      KPLException.process(Res);

      int Count = (int)List.Count;
      List<LedgerDevice> Ret = new List<LedgerDevice>(Count);
      for (int i = 0; i < Count; ++i) {
        IntPtr Dev = Marshal.ReadIntPtr(List.Devs + i*UIntPtr.Size);
        Ret.Add(new LedgerDevice(Dev));
      }
      // Devices are "owned" by the newly created LedgerDevice objects, and
      // will be "deleted" by C# automatically.
      CKPL.KPLLedgerDeviceListFree(ref List, false /* FreeDevices */);
      return Ret;
    }

    public string name() {
      UIntPtr NameLength = (UIntPtr)1024;
      StringBuilder Name = new StringBuilder(1024);
      int Res = CKPL.KPLLedgerDeviceName(Name, ref NameLength, this.Dev);
      KPLException.process(Res);
      return Name.ToString();
    }
  }

  public class KPL {
    private LedgerDevice Dev;
    private KPLPtr Obj;

    public KPL(LedgerDevice Dev, UInt32 TimeoutMS)
    {
      // Keep one ownership in that class
      this.Dev = Dev;

      KPLPtr Obj = (KPLPtr)0;
      int Res = CKPL.KPLFromDevice(ref Obj, this.Dev.Dev, TimeoutMS);
      KPLException.process(Res);
      this.Obj = Obj;
    }

    public void close()
    {
      if (this.Obj != (KPLPtr)0) {
        CKPL.KPLFree(this.Obj);
        this.Obj = (KPLPtr)0;
      }
    }

    ~KPL() {
      close();
    }

    public byte[] getKey(int Slot, UInt32 TimeoutMS) {
      byte[] key = new byte[Constants.KeySize];
      int Res = CKPL.KPLGetKey(this.Obj, (byte)Slot, key, (UIntPtr)Constants.KeySize, TimeoutMS);
      KPLException.process(Res);
      return key;
    }

    public byte[] getKeyFromName(string Name, UInt32 TimeoutMS) {
      byte[] key = new byte[Constants.KeySize];
      int Res = CKPL.KPLGetKeyFromName(this.Obj, Name, key, (UIntPtr)Constants.KeySize, TimeoutMS);
      KPLException.process(Res);
      return key;
    }

    public List<int> getValidSlots(UInt32 TimeoutMS)
    {
      byte[] Slots = new byte[Constants.SlotCount];
      UIntPtr Count = (UIntPtr)0;
      int Res = CKPL.KPLGetValidKeySlots(this.Obj, Slots, ref Count, TimeoutMS);
      KPLException.process(Res);
      List<int> Ret = new List<int>((int)Count);
      for (int i = 0; i < (int)Count; ++i) {
        Ret.Add((int)Slots[i]);
      }
      return Ret;
    }
  }

  internal class CKPL {

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct LedgerDeviceList
    {
      public IntPtr Devs;
      public UIntPtr Count;
    }

    [DllImport(Constants.KPLDllName)]
    public static extern Int32 KPLLedgerDeviceListGet(ref LedgerDeviceList List);

    [DllImport(Constants.KPLDllName)]
    public static extern void KPLLedgerDeviceListFree(ref LedgerDeviceList List, bool FreeDevices);

    [DllImport(Constants.KPLDllName, CharSet = CharSet.Ansi)]
    public static extern Int32 KPLLedgerDeviceName(StringBuilder Name, ref UIntPtr NameLength, LedgerDevicePtr Dev);

    [DllImport(Constants.KPLDllName)]
    public static extern void KPLLedgerDeviceFree(IntPtr Dev);

    [DllImport(Constants.KPLDllName)]
    public static extern int KPLFromDevice(ref KPLPtr Ret, LedgerDevicePtr Dev, UInt32 TimeoutMS);

    [DllImport(Constants.KPLDllName, CharSet = CharSet.Ansi)]
    public static extern int KPLGetKeyFromName(
      KPLPtr Obj, string Name,
      //[MarshalAs(UnmanagedType.LPArray, SizeParamIndex=3)]
      byte[] Out,
      UIntPtr OutLen, UInt32 TimeoutMS);

    [DllImport(Constants.KPLDllName)]
    public static extern int KPLGetKey(
      KPLPtr Obj, byte Slot,
      byte[] Out, UIntPtr OutLen,
      UInt32 TimeoutMS);

    [DllImport(Constants.KPLDllName)]
    public static extern int KPLGetValidKeySlots(KPLPtr Obj, byte[] Out, ref UIntPtr Count, UInt32 TimeoutMS);

    [DllImport(Constants.KPLDllName)]
    public static extern IntPtr KPLErrorStr(int Err);

    [DllImport(Constants.KPLDllName)]
    public static extern void KPLFree(KPLPtr Obj);
  }
}
