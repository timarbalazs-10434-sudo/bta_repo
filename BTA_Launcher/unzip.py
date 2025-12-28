import os
import subprocess
import nitropy
import unreal

#zip_path = ""
#extract_to = ""
#sevenzip = ""

#zip_path = "{zip_path}".replace("\\", "/")
#extract_to = "{extract_to}".replace("\\", "/")


def unzip_split_zip(zip_path, extract_to, sevenzip):
    if not os.path.isfile(zip_path):
        print("ZIP not found:", zip_path)
        return

    os.makedirs(extract_to, exist_ok=True)

    cmd = f'"{sevenzip}" x "{zip_path}" -o"{extract_to}" -y'

    print("Running:", cmd)

    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    print("STDOUT:", result.stdout)
    print("STDERR:", result.stderr)

unzip_split_zip(zip_path, extract_to, sevenzip)
