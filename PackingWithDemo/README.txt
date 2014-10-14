OpenCV 2.4.8的项目模板。
Template for OpenCV 2.4.8 project.

在使用该模板之前，你应该：
1. 新建并设置环境变量"OPENCV_DIR"的值"OpenCV安装目录/build/(x86或者x64)/vc12"；
2. 添加"%OPENCV_DIR%\bin"到系统路径。
Before using this template, you should:
1. set an environment variable "OPENCV_DIR" to "OpenCV_Install_Dir/build/(x86 or x64)/vc12";
2. add "%OPENCV_DIR%\bin" to system PATH.

如果你在visual studio已经打开的情况下执行上述操作，
你可能需要重启visual studio以使系统环境变量设置生效。
If you carried out the instructions above with the visual studio already opened,
you may need to restart the visual studio to let the environment variables go into effect.

你在编译之前也需要根据你的系统以及环境变量的设置在配置管理器中切换平台至win32或者x64。
You should also switch the platform to win32/x64 in "Configuration Manager"
according to your system and environment variable before compiling.