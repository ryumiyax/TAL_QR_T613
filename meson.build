project('TAL_Qr_t613', 'c', 'cpp', version: '1.0.0', default_options : ['c_std=c11', 'cpp_std=c++23'])
warning_level = 3
debug = true
optimization = 3
b_lto = true
b_pgo = 'use'
cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.set_override_option('cpp_std', 'c++23')

cpp = meson.get_compiler('cpp')

add_project_arguments(
	cpp.get_supported_arguments(
		'-D_WIN32_WINNT=_WIN32_WINNT_WIN10',
	),
	language: 'cpp',
)

add_project_link_arguments(
	cpp.get_supported_arguments(
		'-static',
		#'-s',
		'-lws2_32',
		'-lssp',
		'-lntdll',
        # '-lquartz',
        # '-lstrmiids',
        # '-lole32',
        # '-loleaut32',
        # '-luuid',
	),
	language: 'cpp',
)

tomlc99 = subproject('tomlc99')
opt_var.add_cmake_defines({'BUILD_EXAMPLES': false})
zxing_proj = cmake.subproject('zxing', options: opt_var)
zxing_dep = zxing_proj.dependency('ZXing')

library(
	'qr_t613',
	link_with: [
		tomlc99.get_variable('tomlc99_lib'),
	],
	link_args : '-Wl,--allow-multiple-definition',
	include_directories: [
		'src',
		tomlc99.get_variable('tomlc99_inc'),
	],
	dependencies: [
		zxing_dep,
	],
	sources : [
		'src/dllmain.cpp',
        'src/scanner/qrcode.cpp',
        # 'src/camera/videoInput.cpp',
	],
	name_prefix: ''
)
