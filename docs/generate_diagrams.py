"""Produit les diagrammes de classes et de modules (notation UML) au format SVG.

Les deux vues d'ensemble sont décrites à la main à la fin de ce fichier : les modifier dès que
les classes ou leurs liens changent. Les diagrammes détaillés, des données et des modules sont
lus dans le code par uml_details.py. Dans tous les cas, lancer depuis la racine du projet :
python docs/generate_diagrams.py
"""
import io, os, sys
from xml.sax.saxutils import escape

sys.dont_write_bytecode = True

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__))).replace(os.sep, "/")
OUT = ROOT + "/docs/images/"
LINE = "#3b4a5a"

class Diagram:
    def __init__(self, width, height, title):
        self.width, self.height, self.title = width, height, title
        self.bands, self.edges, self.boxes, self.labels = [], [], [], []
        self.rects = {}

    def band(self, y, h, title, fill, stroke):
        self.bands.append(
            f'<rect x="10" y="{y}" width="{self.width - 20}" height="{h}" rx="10" fill="{fill}" stroke="{stroke}"/>'
            f'<text x="52" y="{y + 22}" class="band" fill="{stroke}">{escape(title)}</text>')

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
d = Diagram(1400, 965, "Diagramme de classes : affichage, interface et lien avec le moteur")
d.band(10, 170, "AFFICHAGE (Windows)", "#fff6ea", "#a8641a")
d.band(192, 510, "INTERFACE (C++ pur)", "#f0f6ff", "#2b5f99")
d.band(714, 180, "MOTEUR (C++ pur)", "#eefaf0", "#2c7a3f")

d.box("Win32Input", 60, 55, 220, ["Read()"])
d.box("Win32Window", 350, 55, 220, ["ProcessEvents()", "TakeKeys(), TakeWheel()"])
d.box("Win32Renderer", 780, 55, 220, ["DrawHome()", "DrawSimulation()"])
d.box("Canvas", 1070, 55, 130, ["Fill(), Write()", "Button(), Present()"])

d.box("InputSource", 60, 232, 220, ["Read()"], "interface", "interface")
d.box("UiController", 450, 232, 240, ["Handle() : UiAction", "écran affiché, case choisie"])
d.box("View", 900, 232, 220, ["DrawHome()", "DrawSimulation()"], "interface", "interface")
d.box("SettingEditor", 310, 370, 160, ["saisie d'un réglage"])
d.box("FlatView", 188, 370, 106, ["vue 2D"])
d.box("OrbitCamera", 490, 370, 160, ["vue 3D"])
d.box("ChartSelection", 670, 370, 160, ["graphiques choisis"], "structure")
d.box("Application", 450, 490, 240, ["RunFrame()", "applique les actions"])

d.box("SimulationRunner", 450, 610, 240, ["thread de calcul", "Request(), State()"])

d.box("SimulationClock", 150, 750, 220, ["vitesse et pause"])
d.box("Simulation", 450, 750, 240, ["Step() : une génération"])
d.box("SimulationSnapshot", 760, 750, 200, ["photo à afficher"], "structure")
d.box("SimulationSettings", 1040, 750, 240, ["réglages choisis"], "structure")

# affichage -> interfaces et objets partagés
d.edge("implement", [(210, 113), (210, 232)])
d.edge("implement", [(960, 131), (960, 232)])
d.edge("assoc", [(280, 80), (350, 80)])
d.edge("assoc", [(250, 113), (250, 160), (480, 160), (480, 232)], "commande", (292, 153))
d.edge("assoc", [(830, 131), (830, 160), (660, 160), (660, 232)], "lit l'état", (720, 153))
d.edge("depend", [(780, 80), (570, 80)], "prend sa taille et son identifiant", (675, 72), "middle")
d.edge("depend", [(1000, 80), (1070, 80)], "crée", (1035, 72), "middle")
d.edge("depend", [(60, 95), (18, 95), (18, 778), (150, 778)], "lit les vitesses", (46, 770))
# parties du contrôleur
d.edge("compose", [(510, 308), (510, 340), (390, 340), (390, 370)])
d.edge("compose", [(570, 308), (570, 370)])
d.edge("compose", [(630, 308), (630, 340), (750, 340), (750, 370)])
d.edge("compose", [(480, 308), (480, 325), (241, 325), (241, 370)])
# en relief, le clic en 3D suit les hauteurs de la carte affichée
d.edge("depend", [(620, 428), (620, 462), (722, 462), (722, 725), (780, 725), (780, 750)], "vise sa carte", (630, 456))
# application
d.edge("assoc", [(450, 515), (170, 515), (170, 304)], "reçoit les commandes", (182, 507))
d.edge("assoc", [(690, 515), (1010, 515), (1010, 322)], "fait dessiner", (998, 507), "end")
d.edge("compose", [(500, 564), (500, 590), (260, 590), (260, 750)], "1", (268, 740))
d.edge("compose", [(570, 564), (570, 610)], "0..1", (578, 598))
d.edge("compose", [(640, 564), (640, 578), (1100, 578), (1100, 750)], "1", (1108, 740))
# le thread possède la simulation et publie ses photos
d.edge("compose", [(570, 686), (570, 750)], "1", (578, 740))
d.edge("depend", [(690, 648), (800, 648), (800, 750)], "publie", (730, 640))
d.edge("depend", [(690, 778), (760, 778)], "copie", (725, 770), "middle")
# la simulation et les réglages ; ce que l'interface fait des objets du moteur
d.edge("compose", [(640, 808), (640, 828), (1050, 828), (1050, 808)], "1", (1030, 823), "end")
d.edge("depend", [(900, 290), (875, 290), (875, 750)], "dessine", (883, 700))
d.edge("depend", [(1120, 280), (1220, 280), (1220, 750)], "dessine", (1228, 440))
d.edge("depend", [(690, 262), (855, 262), (855, 725), (1065, 725), (1065, 750)], "modifie", (847, 360), "end")
d.edge("depend", [(400, 428), (400, 848), (1080, 848), (1080, 808)], "modifie", (408, 843))
d.edge("depend", [(60, 262), (34, 262), (34, 872), (1130, 872), (1130, 808)], "le modifie", (46, 867))
d.legend(915)
d.save("classes_application.svg")

# ====================================================== 2. le moteur
d = Diagram(1480, 1035, "Diagramme de classes : le moteur de simulation")
d.box("SimulationSettings", 60, 40, 250, ["eau, obstacles, relief, repousse, soif", "réglages de chaque espèce"], "structure")
d.box("Simulation", 600, 40, 280, ["Step()", "AddAnimal(), Seed()", "Animals(), Count()"])
d.box("SimulationStats", 1150, 40, 260, ["RecordBirth(), RecordDeath()", "History(), Total()"])

d.box("World", 60, 300, 250, ["largeur, hauteur", "GetCell(), IsWalkable()"])
d.box("Animal", 560, 300, 310, ["identifiant, âge, énergie", "position, direction, trajet", "caractéristiques et réglages",
                                 "Advance()", "Steer()  (abstraite)", "Act()  (abstraite)"], "abstraite", "abstract")
d.box("PopulationSample", 1150, 300, 260, ["effectifs, herbe", "énergie, événements"], "structure")

d.box("Cell", 60, 560, 250, ["terrain, herbe de 0 à 100", "hauteur de 0 à 4"])
d.box("WaterRoutes", 60, 690, 250, ["pas jusqu'à la berge", "CanDrink(), NextStep()"])
d.box("Herbivore", 500, 560, 190, ["broute", "peut fuir"], "abstraite", "abstract")
d.box("Deer", 500, 695, 90, ["herbe haute"])
d.box("Rabbit", 600, 695, 90, ["herbe rase"])
d.box("Predator", 740, 560, 190, ["poursuit", "capture une proie"], "abstraite", "abstract")
d.box("Fox", 740, 695, 90, ["lapins"])
d.box("Wolf", 840, 695, 90, ["meute"])
d.box("Habitat", 1150, 560, 260, ["NearestPrey(), NearestMate()", "ThreatsNear(), MatesNear()", "TowardWater()", "CatchPrey(), ShareMeal()"], "interface", "interface")

d.box("SpeciesInfo", 500, 800, 430, ["nom, valeurs de départ, create()",
                                     "catalogue : SpeciesCatalog(), SpeciesOf()"], "structure")
d.box("GridHabitat", 1150, 800, 260, ["animaux rangés par case", "Separate(), FormCouples()"])

# possessions
d.edge("compose", [(600, 80), (310, 80)], "1", (322, 72))
d.edge("compose", [(880, 70), (1150, 70)], "1", (1132, 62))
d.edge("compose", [(640, 134), (640, 200), (185, 200), (185, 300)], "1", (195, 290))
d.edge("compose", [(735, 134), (735, 300)], "0..*", (744, 288))
d.edge("compose", [(1300, 116), (1300, 300)], "0..*  (une par génération)", (1292, 200), "end")
d.edge("compose", [(185, 376), (185, 560)], "1..*", (195, 548))
# héritage : les deux espèces partagent la même flèche
d.edge("inherit", [(595, 560), (595, 515), (715, 515), (715, 462)])
d.edge("inherit", [(835, 560), (835, 515), (715, 515), (715, 462)])
d.edge("inherit", [(545, 695), (545, 674), (595, 674), (595, 650)])
d.edge("inherit", [(645, 695), (645, 674), (595, 674), (595, 650)])
d.edge("inherit", [(785, 695), (785, 674), (835, 674), (835, 650)])
d.edge("inherit", [(885, 695), (885, 674), (835, 674), (835, 650)])
d.edge("implement", [(1280, 800), (1280, 686)])
# la simulation
d.edge("depend", [(880, 122), (960, 122), (960, 240), (1200, 240), (1200, 300)], "mesure", (1040, 232))
d.edge("depend", [(880, 98), (1100, 98), (1100, 815), (1150, 815)], "crée", (1108, 770))
d.edge("depend", [(600, 108), (475, 108), (475, 815), (500, 815)], "consulte", (467, 775), "end")
# les chemins vers l'eau
d.edge("compose", [(610, 134), (610, 160), (488, 160), (488, 722), (310, 722)], "1", (320, 714))
d.edge("depend", [(310, 748), (332, 748), (332, 370), (310, 370)], "mesure à la création", (326, 668), "end")
d.edge("assoc", [(1240, 876), (1240, 912), (185, 912), (185, 766)], "suit les chemins", (195, 904))
# les animaux et leur entourage
d.edge("depend", [(560, 335), (310, 335)], "se déplace sur", (430, 327), "middle")
d.edge("depend", [(870, 340), (945, 340), (945, 590), (1150, 590)], "perçoit par", (937, 505), "end")
d.edge("depend", [(1175, 560), (1175, 480), (420, 480), (420, 355), (310, 355)], "donne accès à", (755, 496))
d.edge("depend", [(1150, 625), (985, 625), (985, 440), (870, 440)], "désigne", (880, 432))
d.edge("depend", [(500, 620), (310, 620)], "mange l'herbe de", (390, 612), "middle")
# ce que GridHabitat garde et lit
d.edge("assoc", [(1150, 850), (1035, 850), (1035, 400), (870, 400)], "contient", (1043, 700))
d.edge("depend", [(1150, 833), (1010, 833), (1010, 668), (915, 668), (915, 695)], "lit SharingDistance", (1004, 660), "end")
d.edge("assoc", [(1200, 876), (1200, 945), (42, 945), (42, 355), (60, 355)], "garde la carte", (60, 937))
d.edge("assoc", [(1410, 822), (1448, 822), (1448, 78), (1410, 78)], "y compte les captures", (1440, 780), "end")
# le catalogue : fabrique les animaux, fournit les valeurs d'origine
d.edge("depend", [(545, 800), (545, 753)], "crée", (553, 782))
d.edge("depend", [(645, 800), (645, 753)], "crée", (653, 782))
d.edge("depend", [(785, 800), (785, 753)], "crée", (793, 782))
d.edge("depend", [(885, 800), (885, 753)], "crée", (893, 782))
d.edge("depend", [(60, 95), (24, 95), (24, 845), (500, 845)], "y prend les valeurs de départ", (56, 835))
d.legend(985)
d.save("classes_moteur.svg")

# ====================================================== 3. détail, données et modules, lus dans le code
import uml_details
uml_details.generate(Diagram, ROOT)
print("OK")
