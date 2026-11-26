"""Filtre Doxygen : retire la syntaxe des modules C++20 avant l'analyse.

Doxygen analyse mal `export` et `import`. Chaque ligne concernée est vidée ou allégée,
sans changer le nombre de lignes : les numéros de ligne de la documentation restent justes.
Les fichiers du projet ne sont pas modifiés.
"""
import re
import sys


def clean(line: str) -> str:
    if re.match(r"\s*(export\s+)?(module|import)\b[^()]*;\s*$", line):
        return "\n"
    return re.sub(r"^(\s*)export\s+(?=namespace\b)", r"\1", line)


def main() -> None:
    with open(sys.argv[1], encoding="utf-8-sig") as source:
        sys.stdout.buffer.write("".join(clean(line) for line in source).encode("utf-8"))


if __name__ == "__main__":
    main()
