# NoMoreCrash

NoMoreCrash is a Windows CLI tool designed to disable Win32 exception handling for a target process. 

It is excellent at handling artihmatic exceptions (like dividing by 0) or out-of-bounds writes/reads. 

The tool also tries to overwrite some function like std::abort, terminate or win32's ExitProcess. This only works when compiled with the same runtime as the target application. (Can be disabled with the TRY_OVERWRITE_ABORT in Payload.dll)

It also tries to break loop that often cause exceptions (very funny) by patching some JMP, JNE, JE instructions with NOOP while the application is trying to crash. This works surprisingly well and leads to funny results when used on game engines. (Can be disabled with TRY_DEFEAT_LOOPS in Payload.dll)

You can take a look at the "CrashTest" solution, this tool will make that program execute everything as if the world was ok and quit with exit code 0.

The tool requires the same or higher privileges than the target process. For instance, you need user-level privileges to target processes like `explorer.exe` but administrator rights if you're targeting system processes (which is not recommended).

Also: Windows Defender really doesn't like this program. You may need to disable real-time protection while building it.

**Warning:**  
By using this tool, you assume full responsibility for any damage to your system or data loss that may occur before, during, or after its use. I do **not** recommend running this on programs where you cannot afford to lose data.

---

## Usage

1. **Target by Process ID (PID):**

   ```cmd
   NoMoreCrash <PID>
   ```

   Example:  
   ```cmd
   NoMoreCrash 12345
   ```

2. **Target by Process Name:**

   ```cmd
   NoMoreCrash --im <ProcessName.exe>
   ```

   Example:  
   ```cmd
   NoMoreCrash --im Notepad.exe
   ```

---

## Compilation

### Prerequisites:
To compile NoMoreCrash, you'll need **Visual Studio 2022** with C++ support or later (it may also work with earlier versions).

### Build Instructions:

1. **Build the Payload Project:**
   - After building the Payload project, check the file size of `Payload.dll`.
   - Insert the correct file size into the appropriate location (begin of Main.cpp of Injector Preject, DEFINE "PAYLOAD_SIZE") (use the release size for the release build and the debug size for the debug build).

2. **Build the Injector Project:**
   - This will automatically append the `Payload.dll` to the `Injector.exe`, creating a single executable.
   - The resulting file will be named `NoMoreCrash.exe`.

3. **Final Step:**
   - You’ll now have the `NoMoreCrash.exe` file. The `Injector.exe` and `Payload.dll` can now be safely deleted.

---

## Why?

The idea for NoMoreCrash came about one evening while I was doing some memory corruption experiments with a colleague in the Unity game engine runtime. More often than not, the engine would crash, which wasn’t what we wanted. This tool was created as a way to address that issue and prevent the engine from crashing in these situations.

Also: It's kinda funny to see how programs die without crashing.
---
