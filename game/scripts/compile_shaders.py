import os
import subprocess
import sys

base_dir = sys.argv[1]
target_dir = sys.argv[2]

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

# Replace the file listing with recursive search
for root, dirs, files in os.walk(base_dir):
	for f in files:
		if f.endswith('.hlsl'):
			filename = os.path.splitext(f)[0]
			full_path = os.path.join(root, f)
			
			# Preserve the subdirectory structure in the target directory
			rel_path = os.path.relpath(root, base_dir)
			output_dir = os.path.join(target_dir, rel_path)
			os.makedirs(output_dir, exist_ok=True)
			
			vs_output_path = os.path.join(output_dir, filename + '.vs_c')
			ps_output_path = os.path.join(output_dir, filename + '.ps_c')
			gs_output_path = os.path.join(output_dir, filename + '.gs_c')

			# Check if the file contains #if COMPILE_GS
			with open(full_path, 'r') as file:
				if '#if COMPILE_GS' in file.read():
					# compile the geometry shader
					r3 = subprocess.run([dxc_path, "-spirv", "-T", "gs_6_0", "-E", "gs_main", "-D", "COMPILE_GS", full_path, "-Fo", gs_output_path])

					if r3.returncode == 0:
						print(f"Compiled GS {full_path} to {output_dir}")
			
			# compile the vertex and pixel shaders
			r1 = subprocess.run([dxc_path, "-spirv", "-T", "vs_6_0", "-E", "vs_main", "-D", "COMPILE_VS", full_path, "-Fo", vs_output_path])
			r2 = subprocess.run([dxc_path, "-spirv", "-T", "ps_6_0", "-E", "ps_main", "-D", "COMPILE_PS", full_path, "-Fo", ps_output_path])
			
			if r1.returncode == 0:
				print(f"Compiled VS {full_path} to {output_dir}")

			if r2.returncode == 0:
				print(f"Compiled PS {full_path} to {output_dir}")

			if (r1.returncode == 0 and r2.returncode == 0):
				print(f"Compiled {full_path} to {output_dir}")
			else:    
				print(f"Failed to compile {full_path}")


	