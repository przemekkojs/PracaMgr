from pathlib import Path
import shutil

def clear_directory(directory: Path):
    if not directory.exists():
        print(f"Folder nie istnieje: {directory}")
        return

    for item in directory.iterdir():
        try:
            if item.is_dir():
                shutil.rmtree(item)
            else:
                item.unlink()
        except Exception as e:
            print(f"Nie udało się usunąć {item}: {e}")

results_dir = Path("../Results")
reports_dir = Path("../Reports")

clear_directory(results_dir)
clear_directory(reports_dir)

print("Foldery ../Results i ../Reports zostały wyczyszczone.")