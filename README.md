#### STM32-F1系列

###### 源码编译

```bash
# 配置 arm-none-eabi-gcc.exe 环境变量
# 配置 STM32_Programmer_CLI.exe 环境变量
# 配置 CMakePresets.json / (CMakeUserPresets.json)
cmake --preset release					# 创建build目录并配置
cmake --build build/release				# 编译
# 连接调试器后
cmake --build .\build\release --target flash	# 编译下载
```

