@echo off
FOR %%i IN (*.vert) DO (
    .\..\..\deps\vulkan\Bin32\glslc.exe %%i -o %%~ni_vert.spv
)

FOR %%i IN (*.frag) DO (
    .\..\..\deps\vulkan\Bin32\glslc.exe %%i -o %%~ni_frag.spv
)

pause