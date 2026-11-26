"""Diagrammes détaillés, des données et des modules, construits à partir du code.

Rien n'est recopié à la main : un type ou un membre ajouté apparaît à la prochaine génération.
"""
from xml.sax.saxutils import escape

import uml_reader

LINE = "#3b4a5a"
COLUMN, GAP, MARGIN = 470, 20, 20
ROW = 17          # hauteur d'une ligne de texte
LIMIT = 68        # nombre de caractères avant de passer à la ligne

CLASS_DIAGRAMS = [
    ("detail_classes_moteur.svg", "Classes du moteur : tous les membres",
     ("src/world/", "src/animals/", "src/simulation/")),
    ("detail_classes_interface.svg", "Classes de l'interface : tous les membres", ("src/ui/",)),
    ("detail_classes_affichage.svg", "Classes de l'affichage : tous les membres", ("src/render/",)),
]
DATA_DIAGRAMS = [
    ("donnees_moteur.svg", "Données du moteur : énumérations, structures et alias",
     ("src/world/", "src/animals/", "src/simulation/")),
    ("donnees_interface_affichage.svg", "Données de l'interface et de l'affichage",
     ("src/ui/", "src/render/")),
]
LAYERS = [
    ("MOTEUR", ("src/world/", "src/animals/", "src/simulation/"), "#eefaf0", "#2c7a3f"),
    ("INTERFACE", ("src/ui/",), "#f0f6ff", "#2b5f99"),
    ("AFFICHAGE", ("src/render/",), "#fff6ea", "#a8641a"),
]


def wrap(text):
    """Découpe une ligne trop longue, de préférence après une virgule ou une espace."""
    lines = []
    while len(text) > LIMIT:
        cut = text.rfind(", ", 20, LIMIT)
        cut = cut + 1 if cut > 0 else text.rfind(" ", 20, LIMIT)
        if cut <= 0:
            cut = LIMIT
        lines.append(text[:cut].rstrip())
        text = "    " + text[cut:].lstrip()
    lines.append(text)
    return lines


def member_rows(members, visibility=True):
    """Transforme des membres en lignes (texte, souligné, italique)."""
    rows = []
    for member in members:
        prefix = f"{member.visibility} " if visibility else ""
        rows += [(line, member.static, member.abstract) for line in wrap(prefix + member.text)]
    return rows


def missing_methods(item, by_name):
    """Noms des méthodes abstraites, propres ou héritées, que la classe ne définit pas."""
    missing = set()
    base = by_name.get(item.bases)
    if base:
        missing = missing_methods(base, by_name)
    missing -= {m.name for m in item.members if m.kind == "function" and not m.abstract}
    return missing | {m.name for m in item.members if m.abstract}


def type_box(item, by_name):
    """Prépare la boîte d'une classe, d'une structure, d'une énumération ou d'un alias."""
    own = any(m.abstract for m in item.members)
    interface = own and all(m.abstract or m.name.startswith("~") for m in item.members)
    abstract = bool(missing_methods(item, by_name))
    stereotype = {"enum": "énumération", "struct": "structure", "alias": "alias"}.get(item.kind)
    kind = "class"
    if item.kind == "class" and abstract:
        stereotype, kind = ("interface", "interface") if interface else ("abstraite", "abstract")
    if item.local:
        file = item.path.rsplit("/", 1)[1]
        stereotype, kind = f"{stereotype + ', ' if stereotype else ''}interne à {file}", "internal"
    elif item.kind == "enum":
        kind = "abstract"
    elif item.kind in ("struct", "alias"):
        kind = "class"
    title = item.name + (f"<{item.template.split()[-1]}>" if item.template else "")
    if item.bases and item.kind != "alias":
        link = "d'" if item.bases[0] in "AEIOU" else "de "
        title += f"  : hérite {link}{item.bases}"
    if item.kind == "enum":
        compartments = [[(line, False, False) for line in wrap(", ".join(item.values))]]
    elif item.kind == "alias":
        compartments = [[(line, False, False) for line in wrap("= " + item.bases)]]
    else:
        attributes = [m for m in item.members if m.kind == "attribute"]
        functions = [m for m in item.members if m.kind == "function"]
        compartments = [member_rows(group) for group in (attributes, functions) if group]
    return title, stereotype, kind, compartments


def height(box):
    """Hauteur d'une boîte préparée."""
    _, stereotype, _, compartments = box
    return (44 if stereotype else 30) + sum(len(rows) * ROW + 10 for rows in compartments)


def draw(diagram, box, x, y):
    """Dessine une boîte préparée à la position donnée."""
    title, stereotype, kind, compartments = box
    header, total = 44 if stereotype else 30, height(box)
    head = {"class": "#dbe7f3", "interface": "#e9defa", "abstract": "#d8f0dc", "internal": "#eceff2"}[kind]
    dash = ' stroke-dasharray="6 4"' if kind == "internal" else ""
    parts = [f'<g class="box" data-width="{COLUMN}">'
             f'<rect x="{x}" y="{y}" width="{COLUMN}" height="{total}" rx="5" fill="#ffffff" stroke="{LINE}" stroke-width="1.4"{dash}/>',
             f'<rect x="{x + 1}" y="{y + 1}" width="{COLUMN - 2}" height="{header - 1}" rx="4" fill="{head}"/>']
    center = x + COLUMN / 2
    if stereotype:
        parts.append(f'<text x="{center}" y="{y + 17}" class="stereo">«{escape(stereotype)}»</text>')
    style = "name abstract" if kind == "interface" or stereotype == "abstraite" else "name"
    parts.append(f'<text x="{center}" y="{y + header - 10}" class="{style}">{escape(title)}</text>')
    top = y + header
    for rows in compartments:
        parts.append(f'<line x1="{x}" y1="{top}" x2="{x + COLUMN}" y2="{top}" stroke="{LINE}"/>')
        for index, (text, static, abstract) in enumerate(rows):
            extra = ' text-decoration="underline"' if static else ""
            extra += ' font-style="italic"' if abstract else ""
            indent = len(text) - len(text.lstrip(" "))
            parts.append(f'<text x="{x + 10 + indent * 6}" y="{top + 17 + index * ROW}" class="member"{extra}>'
                         f'{escape(text.strip())}</text>')
        top += len(rows) * ROW + 10
    diagram.boxes.append("".join(parts) + "</g>")


def place(boxes, columns, top):
    """Range chaque boîte dans la colonne la moins remplie ; rend les positions et le bas."""
    bottoms, positions = [top] * columns, []
    for box in boxes:
        column = bottoms.index(min(bottoms))
        positions.append((box, MARGIN + column * (COLUMN + GAP), bottoms[column]))
        bottoms[column] += height(box) + GAP
    return positions, max(bottoms)


def save(diagram_class, filename, title, boxes, note):
    """Enregistre un diagramme fait d'une grille de boîtes."""
    columns = min(3, max(2, len(boxes)))
    width = 2 * MARGIN + columns * COLUMN + (columns - 1) * GAP
    positions, bottom = place(boxes, columns, 50)
    diagram = diagram_class(width, bottom + 30, title)
    diagram.labels.append(f'<text x="{MARGIN}" y="30" class="legendtitle">{escape(title)}</text>')
    for box, x, y in positions:
        draw(diagram, box, x, y)
    diagram.labels.append(f'<text x="{MARGIN}" y="{bottom + 12}" class="legend">{escape(note)}</text>')
    diagram.save(filename)


def module_box(module):
    """Prépare la boîte d'un module : ses imports et ses déclarations hors classes."""
    compartments = []
    if module.imports:
        imports = ", ".join(name.replace("ecosim.", "") for name in module.imports)
        compartments.append([(line, False, False) for line in wrap("importe : " + imports)])
    declarations = module.constants + module.functions
    if declarations:
        compartments.append(member_rows(declarations, visibility=False))
    header = module.name.endswith(".hpp")
    return module.name, "fichier d'en-tête" if header else "module", "internal" if header else "class", compartments


def save_modules(diagram_class, modules):
    """Enregistre le diagramme des modules, une bande par couche."""
    width = 2 * MARGIN + 3 * COLUMN + 2 * GAP
    top, bands, placed = 50, [], []
    for title, folders, fill, stroke in LAYERS:
        boxes = [module_box(m) for m in modules.values() if m.path.startswith(folders)]
        positions, bottom = place(boxes, 3, top + 36)
        bands.append((top, bottom - top, title, fill, stroke))
        placed += positions
        top = bottom + 14
    title = "Modules : imports, fonctions libres et constantes"
    diagram = diagram_class(width, top + 20, title)
    for band in bands:
        diagram.band(*band)
    diagram.labels.append(f'<text x="{MARGIN}" y="30" class="legendtitle">{title}</text>')
    for box, x, y in placed:
        draw(diagram, box, x, y)
    diagram.save("modules.svg")


def generate(diagram_class, root):
    """Lit le code et écrit tous les diagrammes détaillés."""
    modules, types = uml_reader.read_project(root)
    paths = {name: module.path for name, module in modules.items()}
    by_name = {t.name: t for t in types}
    placed = 0
    for diagrams, wanted, note in (
            (CLASS_DIAGRAMS, ("class",),
             "+ public  ·  # protégé  ·  - privé  ·  souligné : statique  ·  italique : abstrait"),
            (DATA_DIAGRAMS, ("enum", "struct", "alias"),
             "+ public  ·  - privé  ·  « interne à » : type propre à un fichier .cpp")):
        for filename, title, folders in diagrams:
            chosen = [t for t in types if t.kind in wanted and paths[t.module].startswith(folders)]
            placed += len(chosen)
            save(diagram_class, filename, title, [type_box(t, by_name) for t in chosen], note)
    if placed != len(types):
        raise SystemExit(f"{len(types) - placed} type(s) absent(s) des diagrammes")
    save_modules(diagram_class, modules)
    print(f"{len(types)} types et {len(modules)} modules lus dans le code")
