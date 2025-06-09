@echo off
cd %~p0

REM Looks for conan in PATH and returns error code: 0 if command exists in PATH, 1 if not.
WHERE /q conan


IF %ERRORLEVEL% == 0 (
	REM If conan is in PATH then execute it
	REM OpenGL
	conan install conanfile.txt -if ../deps/RelWithDebInfoPC -s build_type=Release -s compiler.runtime=MD --build=missing
	conan install conanfile.txt -if ../deps/ReleasePC -s build_type=Release -s compiler.runtime=MD --build=missing
	conan install conanfile.txt -if ../deps/DebugPC -s build_type=Debug -s compiler.runtime=MDd --build=missing
	REM Xbox
	conan install conanfile.txt -if ../deps/RelWithDebInfoXbox -s build_type=Release -s compiler.runtime=MD --build=missing
	conan install conanfile.txt -if ../deps/ReleaseXbox -s build_type=Release -s compiler.runtime=MD --build=missing
	conan install conanfile.txt -if ../deps/DebugXbox -s build_type=Debug -s compiler.runtime=MDd --build=missing
) ELSE (
	REM If not, then search for .exe path
	REM OpenGL
	C:\Users\%USERNAME%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts\conan install conanfile.txt -if ../deps/RelWithDebInfoPC -s build_type=Release -s compiler.runtime=MD --build=missing
	C:\Users\%USERNAME%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts\conan install conanfile.txt -if ../deps/ReleasePC -s build_type=Release -s compiler.runtime=MD --build=missing
	C:\Users\%USERNAME%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts\conan install conanfile.txt -if ../deps/DebugPC -s build_type=Debug -s compiler.runtime=MDd --build=missing
	REM Xbox
	C:\Users\%USERNAME%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts\conan install conanfile.txt -if ../deps/RelWithDebInfoXbox -s build_type=Release -s compiler.runtime=MD --build=missing
	C:\Users\%USERNAME%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts\conan install conanfile.txt -if ../deps/ReleaseXbox -s build_type=Release -s compiler.runtime=MD --build=missing
	C:\Users\%USERNAME%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.11_qbz5n2kfra8p0\LocalCache\local-packages\Python311\Scripts\conan install conanfile.txt -if ../deps/DebugXbox -s build_type=Debug -s compiler.runtime=MDd --build=missing
)

pause