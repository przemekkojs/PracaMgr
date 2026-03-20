import shutil

source_files = ["48.wav", "48_attack.wav", "48_release.wav"]

for i in range(49, 73):
    for file in source_files:
        new_file = file.replace("48", str(i))
        shutil.copy(file, new_file)

