"""Produit les deux diagrammes de classes (notation UML) au format SVG.

Les classes et les liens sont décrits à la main à la fin de ce fichier : les modifier dès que
le code change, puis lancer depuis la racine du projet :  python docs/generate_diagrams.py
"""
import io, os
from xml.sax.saxutils import escape

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "images", "")
LINE = "#3b4a5a"

class Diagram:
    def __init__(self, width, height, title):
        self.width, self.height, self.title = width, height, title
        self.bands, self.edges, self.boxes, self.labels = [], [], [], []
        self.rects = {}

    def band(self, y, h, title, fill, stroke):
        self.bands.append(
            f'<rect x="10" y="{y}" width="{self.width - 20}" height="{h}" rx="10" fill="{fill}" stroke="{stroke}"/>'
            f'<text x="24" y="{y + 22}" class="band" fill="{stroke}">{escape(title)}</text>')

    def box(self, name, x, y, w, lines=(), stereotype=None, kind="class"):
        header = 44 if stereotype else 30
        h = header + (len(lines) * 18 + 10 if lines else 0)
        head = {"class": "#dbe7f3", "interface": "#e9defa", "abstract": "#d8f0dc", "internal": "#eceff2"}[kind]
        dash = ' stroke-dasharray="6 4"' if kind == "internal" else ""
        parts = [f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="5" fill="#ffffff" stroke="{LINE}" stroke-width="1.4"{dash}/>',
                 f'<rect x="{x + 1}" y="{y + 1}" width="{w - 2}" height="{header - 1}" rx="4" fill="{head}"/>']
        cx = x + w / 2
        if stereotype:
            parts.append(f'<text x="{cx}" y="{y + 17}" class="stereo">«{escape(stereotype)}»</text>')
        style = "name abstract" if kind in ("abstract", "interface") else "name"
        parts.append(f'<text x="{cx}" y="{y + header - 10}" class="{style}">{escape(name)}</text>')
        if lines:
            parts.append(f'<line x1="{x}" y1="{y + header}" x2="{x + w}" y2="{y + header}" stroke="{LINE}"/>')
        for i, text in enumerate(lines):
            parts.append(f'<text x="{x + 10}" y="{y + header + 19 + i * 18}" class="member">{escape(text)}</text>')
        self.boxes.append("".join(parts))
        self.rects[name] = (x, y, w, h)
        return (x, y, w, h)

    def edge(self, kind, points, label=None, at=None, anchor="start"):
        """kind : inherit, implement (flèche au dernier point), compose (losange au premier),
        assoc, depend (flèche au dernier point)."""
        path = "M " + " L ".join(f"{x} {y}" for x, y in points)
        dashed = ' stroke-dasharray="7 5"' if kind in ("implement", "depend") else ""
        marker = {"inherit": 'marker-end="url(#triangle)"', "implement": 'marker-end="url(#triangle)"',
                  "compose": 'marker-start="url(#diamond)"', "assoc": 'marker-end="url(#arrow)"',
                  "depend": 'marker-end="url(#arrow)"'}[kind]
        self.edges.append(f'<path d="{path}" fill="none" stroke="{LINE}" stroke-width="1.5"{dashed} {marker}/>')
        if label:
            lx, ly = at
            self.labels.append(f'<text x="{lx}" y="{ly}" class="label" text-anchor="{anchor}">{escape(label)}</text>')

    def legend(self, y):
        items = [("inherit", "héritage"), ("implement", "implémente une interface"),
                 ("compose", "possède (composition)"), ("assoc", "garde une référence"), ("depend", "utilise")]
        self.boxes.append(f'<text x="24" y="{y + 5}" class="legendtitle">Légende</text>')
        for index, (kind, text) in enumerate(items):
            x, line = 110 + index % 3 * 320, y + index // 3 * 28
            self.edge(kind, [(x, line), (x + 54, line)])
            self.labels.append(f'<text x="{x + 64}" y="{line + 5}" class="legend">{escape(text)}</text>')

    def save(self, filename):
        svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {self.width} {self.height}" role="img" aria-label="{escape(self.title)}">
<title>{escape(self.title)}</title>
<style>
 text {{ font-family: "Segoe UI", Arial, sans-serif; fill: #17212b; }}
 .name {{ font-size: 15px; font-weight: 700; text-anchor: middle; }}
 .abstract {{ font-style: italic; }}
 .stereo {{ font-size: 11px; text-anchor: middle; fill: #4a5968; }}
 .member {{ font-size: 12.5px; }}
 .label {{ font-size: 12px; font-style: italic; fill: #3b4a5a; }}
 .band {{ font-size: 14px; font-weight: 700; }}
 .legend {{ font-size: 12.5px; }}
 .legendtitle {{ font-size: 13px; font-weight: 700; }}
</style>
<defs>
 <marker id="triangle" viewBox="0 0 14 14" refX="13" refY="7" markerWidth="14" markerHeight="14" markerUnits="userSpaceOnUse" orient="auto"><path d="M1 1 L13 7 L1 13 Z" fill="#ffffff" stroke="{LINE}" stroke-width="1.4"/></marker>
 <marker id="diamond" viewBox="0 0 18 12" refX="1" refY="6" markerWidth="18" markerHeight="12" markerUnits="userSpaceOnUse" orient="auto"><path d="M1 6 L9 1 L17 6 L9 11 Z" fill="{LINE}"/></marker>
 <marker id="arrow" viewBox="0 0 12 12" refX="11" refY="6" markerWidth="12" markerHeight="12" markerUnits="userSpaceOnUse" orient="auto"><path d="M1 1 L11 6 L1 11" fill="none" stroke="{LINE}" stroke-width="1.6"/></marker>
</defs>
<rect width="{self.width}" height="{self.height}" fill="#ffffff"/>
{"".join(self.bands)}
{"".join(self.edges)}
{"".join(self.boxes)}
{"".join(self.labels)}
</svg>
'''
        os.makedirs(OUT, exist_ok=True)
        io.open(OUT + filename, "w", encoding="utf-8", newline="\n").write(svg)

# ====================================================== 1. de l'application au moteur
d = Diagram(1100, 815, "Diagramme de classes : affichage, interface et lien avec le moteur")
d.band(10, 170, "AFFICHAGE (Windows)", "#fff6ea", "#a8641a")
d.band(192, 400, "INTERFACE (C++ pur)", "#f0f6ff", "#2b5f99")
d.band(604, 140, "MOTEUR (C++ pur)", "#eefaf0", "#2c7a3f")

d.box("Win32Input", 40, 55, 220, ["Read()"])
d.box("Win32Window", 330, 55, 220, ["ProcessEvents()", "TakeKeys(), TakeWheel()"])
d.box("Win32Renderer", 760, 55, 220, ["DrawHome()", "DrawSimulation()"])

d.box("InputSource", 40, 232, 220, ["Read()"], "interface", "interface")
d.box("UiController", 430, 232, 240, ["Handle() : UiAction", "écran affiché, case choisie"])
d.box("View", 840, 232, 220, ["DrawHome()", "DrawSimulation()"], "interface", "interface")
d.box("SettingEditor", 290, 370, 160, ["saisie d'un réglage"])
d.box("OrbitCamera", 470, 370, 160, ["vue 3D"])
d.box("ChartSelection", 650, 370, 160, ["graphiques choisis"])
d.box("Application", 430, 490, 240, ["RunFrame()", "applique les actions"])

d.box("SimulationClock", 90, 640, 220, ["vitesse et pause"])
d.box("Simulation", 430, 640, 240, ["Step() : une génération"])
d.box("SimulationSettings", 780, 640, 230, ["réglages choisis"])

# affichage -> interfaces et objets partagés
d.edge("implement", [(100, 113), (100, 232)])
d.edge("implement", [(950, 131), (950, 232)])
d.edge("assoc", [(260, 80), (330, 80)])
d.edge("assoc", [(200, 113), (200, 160), (460, 160), (460, 232)], "commande", (272, 153))
d.edge("assoc", [(810, 131), (810, 160), (640, 160), (640, 232)], "lit l'état", (700, 153))
d.edge("depend", [(760, 80), (550, 80)], "prend sa taille et son identifiant", (655, 72), "middle")
# parties du contrôleur
d.edge("compose", [(490, 316), (490, 340), (370, 340), (370, 370)])
d.edge("compose", [(550, 316), (550, 370)])
d.edge("compose", [(610, 316), (610, 340), (730, 340), (730, 370)])
# application
d.edge("assoc", [(430, 515), (150, 515), (150, 308)], "reçoit les commandes", (162, 507))
d.edge("assoc", [(670, 515), (950, 515), (950, 326)], "fait dessiner", (938, 507), "end")
d.edge("compose", [(480, 564), (480, 590), (200, 590), (200, 640)], "1", (208, 630))
d.edge("compose", [(550, 564), (550, 640)], "0..1", (558, 630))
d.edge("compose", [(620, 564), (620, 590), (895, 590), (895, 640)], "1", (903, 630))
d.legend(765)
d.save("classes_application.svg")

# ====================================================== 2. le moteur
d = Diagram(1100, 830, "Diagramme de classes : le moteur de simulation")
d.box("SimulationSettings", 40, 40, 250, ["eau, obstacles, repousse", "réglages de chaque espèce"])
d.box("Simulation", 430, 40, 250, ["Step()", "AddAnimal(), Seed()", "Animals(), Count()"])
d.box("SimulationStats", 820, 40, 240, ["RecordBirth(), RecordDeath()", "History(), Total()"])

d.box("World", 40, 270, 250, ["largeur, hauteur", "GetCell(), IsWalkable()"])
d.box("Animal", 400, 260, 310, ["identifiant, âge, énergie", "position, direction, trajet", "traits : AnimalTraits",
                                 "Advance()", "Steer()  (abstraite)", "Act()  (abstraite)"], "abstraite", "abstract")
d.box("PopulationSample", 820, 270, 240, ["effectifs, herbe", "énergie, événements"])

d.box("Cell", 40, 480, 250, ["terrain", "herbe de 0 à 100"])
d.box("Herbivore", 330, 530, 200, ["broute", "peut fuir"])
d.box("Predator", 580, 530, 200, ["chasse", "peut chasser en meute"])
d.box("Habitat", 820, 460, 240, ["NearestPrey()", "HuntersNear()", "CatchPrey(), ShareMeal()"], "interface", "interface")
d.box("GridHabitat", 820, 660, 240, ["animaux rangés par case", "créé à chaque génération"], "interne à Simulation", "internal")

d.edge("compose", [(430, 80), (290, 80)], "1", (302, 72))
d.edge("compose", [(680, 80), (820, 80)], "1", (800, 72))
d.edge("compose", [(470, 134), (470, 190), (170, 190), (170, 270)], "1", (180, 258))
d.edge("compose", [(555, 134), (555, 260)], "0..*", (564, 248))
d.edge("compose", [(940, 116), (940, 270)], "0..*  (une par génération)", (950, 252))
d.edge("compose", [(165, 346), (165, 480)], "1..*", (175, 468))
# héritage : les deux espèces partagent la même flèche
d.edge("inherit", [(430, 530), (430, 480), (555, 480), (555, 422)])
d.edge("inherit", [(680, 530), (680, 480), (555, 480), (555, 422)])
d.edge("depend", [(710, 350), (765, 350), (765, 495), (820, 495)], "perçoit par", (772, 420))
d.edge("implement", [(940, 660), (940, 568)])
d.edge("depend", [(680, 120), (730, 120), (730, 210), (800, 210), (800, 690), (820, 690)], "crée", (736, 202))
d.legend(778)
d.save("classes_moteur.svg")
print("OK")
