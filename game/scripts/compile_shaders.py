import os
import subprocess
import sys

base_dir = sys.argv[1]
target_dir = sys.argv[2]

files = [f for f in os.listdir(base_dir) if f.endswith('.hlsl')]

def find_dxc():
	# get vulkan sdk env
	vulkan_sdk_path = os.environ.get('VULKAN_SDK')
	if not vulkan_sdk_path:
		print("VULKAN_SDK not set")
		return None

	# find dxc
	dxc_path = os.path.join(vulkan_sdk_path, 'Bin', 'dxc.exe')
	if not os.path.exists(dxc_path):
		print("dxc not found")
		return None

	return dxc_path


dxc_path = find_dxc()

for f in files:
	filename = os.path.splitext(f)[0]
	full_path = os.path.join(base_dir, f)

	output_path = os.path.join(target_dir, filename)
	vs_output_path = os.path.join(target_dir, filename + '.vs_c')
	ps_output_path = os.path.join(target_dir, filename + '.ps_c')

	# compile the shader
	subprocess.run([dxc_path, "-spirv", "-T", "vs_6_0", "-E", "vs_main", full_path, "-Fo", vs_output_path])
	subprocess.run([dxc_path, "-spirv", "-T", "ps_6_0", "-E", "ps_main", full_path, "-Fo", ps_output_path])

	print("Compiled " + f + " to " + output_path)
	