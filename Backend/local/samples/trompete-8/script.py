import shutil

source_files = ["60.wav"]

for i in range(36, 96):
    if i == 60:
        continue

    for file in source_files:
        new_file = file.replace("60", str(i))
        shutil.copy(file, new_file)
