import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

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

def compile_shader(dxc_path, full_path, output_path, shader_type, entry_point, define):
    result = subprocess.run([
        dxc_path, "-spirv", "-fvk-use-scalar-layout", "-Zi", "-Od",
        "-T", shader_type, "-E", entry_point, "-D", define,
        full_path, "-Fo", output_path
    ])
    return result.returncode == 0

def process_file(full_path, output_dir, dxc_path):
    filename = os.path.splitext(os.path.basename(full_path))[0]
    vs_output_path = os.path.join(output_dir, filename + '.vs_c')
    ps_output_path = os.path.join(output_dir, filename + '.ps_c')
    gs_output_path = os.path.join(output_dir, filename + '.gs_c')
    cs_output_path = os.path.join(output_dir, filename + '.cs_c')

    tasks = []
    with open(full_path, 'r') as file:
        content = file.read()
        
        if '#if COMPILE_GS' in content:
            tasks.append(('gs_6_0', 'gs_main', 'COMPILE_GS', gs_output_path))
        if '#if COMPILE_CS' in content:
            tasks.append(('cs_6_0', 'cs_main', 'COMPILE_CS', cs_output_path))
        if '#if COMPILE_VS' in content:
            tasks.append(('vs_6_0', 'vs_main', 'COMPILE_VS', vs_output_path))
        if '#if COMPILE_PS' in content:
            tasks.append(('ps_6_0', 'ps_main', 'COMPILE_PS', ps_output_path))

    with ThreadPoolExecutor() as executor:
        futures = []
        for shader_type, entry_point, define, output_path in tasks:
            futures.append(executor.submit(
                compile_shader,
                dxc_path,
                full_path,
                output_path,
                shader_type,
                entry_point,
                define
            ))

        success = all(f.result() for f in futures)
        if success:
            print(f"Finished compiling {full_path} to {output_dir}")
        else:
            print(f"Failed to compile {full_path}")
        print("----------------------")


# main logic
dxc_path = find_dxc()
if not dxc_path:
    sys.exit(1)

with ThreadPoolExecutor() as executor:
    futures = []
    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith('.hlsl'):
                full_path = os.path.join(root, f)
                rel_path = os.path.relpath(root, base_dir)
                output_dir = os.path.join(target_dir, rel_path)
                os.makedirs(output_dir, exist_ok=True)
                
                futures.append(executor.submit(
                    process_file,
                    full_path,
                    output_dir,
                    dxc_path
                ))

    for future in as_completed(futures):
        future.result()
