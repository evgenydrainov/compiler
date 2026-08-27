set PATH=C:\Program Files\LLVM\bin;%PATH%

lld-link /def:kernel32.def /out:kernel32.lib /machine:x64

lld-link main.obj ^
    gamelib.lib ^
    SDL3.lib ^
    kernel32.lib ^
    /SUBSYSTEM:CONSOLE ^
    /LIBPATH:"C:\Users\Username\source\repos\gamelib\build\RelWithDebInfo" ^
    /LIBPATH:"C:\Users\Username\source\repos\gamelib\vendor\SDL3\lib\x64"

pause
