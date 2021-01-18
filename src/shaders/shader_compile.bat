@echo off
setlocal ENABLEDELAYEDEXPANSION

set MAX_LIGHTS=#define MAX_LIGHTS 25
set LIGHT_DATA=struct LightSource{vec4 pos; vec4 lightcolor;};
set LIGHT_DEFINE="#define LIGHT"

FOR %%a IN (glsl\*.vert) DO (
    FOR /f "delims=" %%i IN (%%a) DO (
        set var=%%i
        if "%%i"==%LIGHT_DEFINE% (
            echo %MAX_LIGHTS% >> output.vert
            set var=%LIGHT_DATA%
        )
        echo !var! >> output.vert
    )
    .\..\..\deps\vulkan\Bin32\glslc.exe output.vert -o spir-v\%%~na_vert.spv
    del output.vert
)

FOR %%a IN (glsl\*.frag) DO (
    FOR /f "delims=" %%i IN (%%a) DO (
        set var=%%i
        if "%%i"==%LIGHT_DEFINE% (
            echo %MAX_LIGHTS% >> output.frag
            set var=%LIGHT_DATA%
        )
        echo !var! >> output.frag
    )
    .\..\..\deps\vulkan\Bin32\glslc.exe output.frag -o spir-v\%%~na_frag.spv
    del output.frag
)

pause