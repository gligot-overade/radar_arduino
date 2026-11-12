import sys
from difflib import unified_diff
from pathlib import Path


PAIRS = [
    # Ajoutez ici des paires de fichiers ou de dossiers à comparer :
    # ("chemin/vers/fichier1.txt", "chemin/vers/fichier2.txt"),
    # ("chemin/vers/dossier1", "chemin/vers/dossier2"),
    ("/home/gligot/gligot/radar-sketch/radar/README.md", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/README.md"),
    ("/home/gligot/gligot/radar-sketch/radar/radar.ino", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/radar.ino"),
    ("/home/gligot/gligot/radar-sketch/radar/config.h", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/config.h"),
    ("/home/gligot/gligot/radar-sketch/radar/src/affichage.h", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/affichage.h"),
    ("/home/gligot/gligot/radar-sketch/radar/src/affichage.c", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/affichage.c"),
    ("/home/gligot/gligot/radar-sketch/radar/src/acquisition_signal.c", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/acquisition_signal.c"),
    ("/home/gligot/gligot/radar-sketch/radar/src/acquisition_signal.h", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/acquisition_signal.h"),
    ("/home/gligot/gligot/radar-sketch/radar/src/radar-targets/arduino/hardware_config.h", "/home/gligot/OXITECH_TC321X/build/tc321x/Firmware_OXI/oxitech_light_targets/radar-targets/arduino/hardware_config.h")
]


def color(text, code):
    if sys.stdout.isatty():
        return f"\033[{code}m{text}\033[0m"
    return text


def compare_files(first_path, second_path):
    print(f"\n--- Comparaison : {first_path} / {second_path} ---")
    try:
        first_lines = first_path.read_text(encoding="utf-8").splitlines()
        second_lines = second_path.read_text(encoding="utf-8").splitlines()
    except FileNotFoundError as error:
        print(color(f"Fichier manquant : {error.filename}", "31"), file=sys.stderr)
        return

    differences = unified_diff(
        first_lines,
        second_lines,
        fromfile=str(first_path),
        tofile=str(second_path),
        lineterm="",
    )
    diff_lines = list(differences)
    if not diff_lines:
        print("Les fichiers sont identiques.")
        return

    for line in diff_lines:
        if line.startswith(("--- ", "+++ ")):
            print(color(line, "36"))
        elif line.startswith("-"):
            print(color(line, "31"))
        elif line.startswith("+"):
            print(color(line, "32"))
        elif line.startswith("@@"):
            print(color(line, "36"))
        else:
            print(line)


def compare_directories(first_path, second_path):
    first_files = {
        path.relative_to(first_path): path
        for path in first_path.rglob("*")
        if path.is_file()
    }
    second_files = {
        path.relative_to(second_path): path
        for path in second_path.rglob("*")
        if path.is_file()
    }

    all_relative_paths = sorted(first_files.keys() | second_files.keys())
    if not all_relative_paths:
        print("Les dossiers ne contiennent aucun fichier à comparer.")
        return

    for relative_path in all_relative_paths:
        first_file = first_files.get(relative_path)
        second_file = second_files.get(relative_path)
        if first_file is None:
            print(color(f"Fichier présent uniquement dans {second_path} : {relative_path}", "33"))
        elif second_file is None:
            print(color(f"Fichier présent uniquement dans {first_path} : {relative_path}", "33"))
        else:
            compare_files(first_file, second_file)


def compare_pair(first, second):
    first_path = Path(first)
    second_path = Path(second)
    print(f"\n=== Paire : {first_path} / {second_path} ===")

    missing = [path for path in (first_path, second_path) if not path.exists()]
    if missing:
        for path in missing:
            print(color(f"Chemin manquant : {path}", "31"), file=sys.stderr)
        return

    if first_path.is_dir() and second_path.is_dir():
        compare_directories(first_path, second_path)
    elif first_path.is_file() and second_path.is_file():
        compare_files(first_path, second_path)
    else:
        print(
            color(
                "Erreur : les deux chemins doivent être soit des fichiers, soit des dossiers.",
                "31",
            ),
            file=sys.stderr,
        )


def main():
    if not PAIRS:
        print("Ajoutez au moins une paire de fichiers ou de dossiers dans la liste PAIRS.")
        return

    for first, second in PAIRS:
        compare_pair(first, second)


if __name__ == "__main__":
    main()