@ECHO OFF
FOR /R %%g in (*.glsl) do (
	echo glslangValidator -V %%g -o shaders/%%~ng.spv
	glslangValidator -V %%g -o shaders/%%~ng.spv
)
