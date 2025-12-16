# imports the tkinter module itself and gives it the alias tk
import tkinter as tk
# imports specific submodules directly into your namespac
from tkinter import colorchooser, messagebox
import math

# ---------------------------------------------------------------------------------------
# --------------------------------- Outils géométriques ---------------------------------
# ---------------------------------------------------------------------------------------

def distance(a,b):
    return math.sqrt( ( a[0]-a[1] )**2 + ( b[0]-b[1] )**2 )

def cross(a,b):
    return (a[0]*b[1]) - (a[1]*b[0])

def cross2(a,b,c,d):
    return (b[0]-a[0])*(d[1]-c[1]) - (b[1]-a[1])*(d[0]-c[0])

# Présume que le polygone suit le sens trigonométrique
# Retourne vrai si l'angle (a->b->c) est convexe 
def is_convex(a,b,c):
        return cross2( a, b, a, c ) > 0

# Vérifie si p se situe dans le triangle formé par les points a, b, c
# Utilise les coordonnées barycentriques
def is_point_in_triangle(p, a, b, c):
    # Déterminant du triangle
    detT = (b[1]-c[1])*(a[0]-c[0]) + (c[0]-b[0])*(a[1]-c[1])
    # Coordonnées baycentriques de p
    alpha = ((b[1]-c[1])*(p[0]-c[0]) + (c[0]-b[0])*(p[1]-c[1])) / detT
    beta = ((c[1]-a[1])*(p[0]-c[0]) + (a[0]-c[0])*(p[1]-c[1])) / detT
    gamma = 1.0 - alpha - beta
    return (0 < alpha < 1) and (0 < beta < 1) and (0 < gamma < 1)

# Calcule l'aire signée du polygone ( positive = sens trigonométrique, négative sinon )
# Mets en place la formule du lacet / formule d'aire de Gauss permettant de déterminer l'aire d'un polygone simple
# Vérifie si le polygone est orienté dans le sens trigonométrique à partir du signe de l'aire
# sens trigonométrique -> contre le sens des aiguilles d'une montre -> counter clock wise
def is_counterClockWise(points):
    area = 0

    for i in range(len(points)):
        area += cross( points[i], points[(i+1) % len(points)] ) # somme des produits en croix

    polygon_area = area / 2
    return polygon_area > 0

# Teste si le point p est à l'intérieur du demi-plan défini par (a,b)
# Si le produit en croix est positif, p est "à gauche" de l'arête ab ( à l'intérieur d'un polygone défini dans le sens trigonométrique )
def inside(p, a, b):
    return cross2( a, b, a, p ) >= 0

# Calcul l'intersection entre les segments p1-p2 et a-b
def intersection(p1, p2, p3, p4):
    x1, y1 = p1
    x2, y2 = p2
    x3, y3 = p3
    x4, y4 = p4

    denom = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4)
    if denom == 0: # les segments sont parallèles 
        return p2 # Valeur par défaut

    px = ((x1*y2 - y1*x2)*(x3-x4) - (x1-x2)*(x3*y4 - y3*x4)) / denom
    py = ((x1*y2 - y1*x2)*(y3-y4) - (y1-y2)*(x3*y4 - y3*x4)) / denom
    return (px, py)

# Retourne vrai si le segment p1-p2 est en collision avec la droite définie par p3-p4
def collide(p1, p2, p3, p4):
    if p1 is None:
        return False

    cross_p1= cross2( p3, p4, p3, p1 )
    cross_p2 = cross2( p3, p4, p3, p2 )

    # Si p1 et p2 sont de côtés opposés par rapport à la droite alors il y a collision
    return cross_p1 * cross_p2 < 0

# ---------------------------------------------------------------------------------------
# ----------------------- Marche de Jarvis ( Enveloppe Convexe ) ------------------------
# ---------------------------------------------------------------------------------------

# Calcule l'enveloppe convexe d'un nuage de points
def jarvis_march(points):
    # Une enveloppe convexe a besoin d'au moins 3 points
    if len(points) < 3:
        return points[:] # copie des points

    hull_points = []
    # retourne le point avec la plus petite coordonnées en x car il est obligé d'être dans l'enveloppe
    left_most = min(points, key=lambda p: p[0])
    # point courant dans la boucle
    p = left_most

    while True:
        hull_points.append(p)

        # le point le plus "à gauche" de p
        q = points[0]
        for r in points:
            # On ne traite pas le point courant
            if r == p:
                continue
            
            cross_res = cross2( p, q, p, r )
            # cross_res < 0 : r est plus "à gauche" que q
            # cross_res = 0 : p, q, r sont colinéaires. On met à jour q seulement si r est plus loin que q de p
            if cross_res < 0 or (cross_res == 0 and distance(p,r) > distance(p,q)):
                q = r

        p = q
        # si on est retourné au point de départ, quitter la boucle
        if p == left_most:
            break

    return hull_points

# ---------------------------------------------------------------------------------------
# ----------------- Ear Cutting ( Triangulation de polygone concave ) -------------------
# ---------------------------------------------------------------------------------------

# Retourne une liste de triangles pour un polygone concave
# points : liste de tuples [(x,y), ...], dans le sens trigonométrique
def earcut_triangulation(points):
    triangles = []
    pts = points.copy()

    # Jusqu'à ce qu'il reste qu'un triangle
    while len(pts) > 3:
        n = len(pts)
        ear_found = False

        for i in range(n):
            prev = pts[(i-1) % n]
            curr = pts[i] # ear potentiel
            next = pts[(i+1) % n]

            # un ear doit être convexe et ne doit pas contenir un autre point dans son triangle
            if is_convex(prev, curr, next):
                has_point_inside = False
                for p in pts:
                    if p != prev and p != curr and p != next and is_point_in_triangle(p, prev, curr, next):
                        has_point_inside = True
                        break

                # Si l'ear actuel est valide
                if not has_point_inside: 
                    triangles.append([prev, curr, next])
                    del pts[i]
                    ear_found = True
                    break
        # Si pas de ear a été trouvé, le polygone est soit pas dans le sens trigonométrique soit s'auto-intersecte
        if not ear_found:
            break
    
    # On ajoute le triangle final restant
    if len(pts) == 3:
        triangles.append([pts[0], pts[1], pts[2]])
    
    return triangles

# ---------------------------------------------------------------------------------------
# --------------------------- Fenêtrage Sutherland–Hodgman ------------------------------
# ---------------------------------------------------------------------------------------

# Découpe le polygone PL par le polygone convexe PW
# PL : liste de points [(x,y), ...]
# PW : liste de points [(x,y), ...], F1 = FN3
def sutherland_hodgman(PL, PW):
    N1 = len(PL)
    N3 = len(PW)

    # Pour chaque côté de la fenêtre
    for i in range(N3):  
        Fi = PW[i]
        Fi1 = PW[(i+1)%N3]

        PS = []        # Polygone de sortie pour ce côté
        N2 = 0         # Nombre de points dans PS
        F = PL[0]      # Point initial
        S = None

        # Pour chaque arrête du polygone
        for j in range(N1):
            Pj = PL[j]

            # Si collision entre segment S->Pj et la droite définie par Fi et Fi1
            if collide(S, Pj, Fi, Fi1):
                I = intersection(S, Pj, Fi, Fi1)
                PS.append(I)
                N2 += 1

            # Si Pj est à l'intérieur de polygone
            if inside(Pj, Fi, Fi1):
                PS.append(Pj)
                N2 += 1

            S = Pj  # Passer au point suivant

        # Traitement de la dernière arête
        if N2 > 0:
            if collide(S, F, Fi, Fi1):
                I = intersection(S, F, Fi, Fi1)
                PS.append(I)
                N2 += 1

        # Mise à jour pour le prochain côté de la fenêtre
        PL = PS.copy()
        N1 = N2

    return PL


'''
# Fenêtrage d'un polygone 'polygone' par un polygone 'window' ( convexe ou triangulé )
def sutherland_hodgman(polygone, window):
    output = polygone

    for i in range(len(window)):
        input_list = output
        output = []
        if not input_list:
            break
        
        # Arête actuel ( avec wrapping )
        a = window[i]
        b = window[(i+1) % len(window)]
        s = input_list[-1]

        # Itère sur chaque arête s -> e
        for e in input_list:
            if inside(e, a, b): 
                if not inside(s, a, b): 
                    # rentre dans la fenêtre
                    output.append(intersection(s, e, a, b))
                # est dans la fenêtre
                output.append(e) 

            elif inside(s, a, b): 
                # quitte la fenêtre
                output.append(intersection(s, e, a, b))

            # est en dehors de la fenêtre ( ne rien faire )
            s = e

    return output
'''

# ---------------------------------------------------------------------------------------
# --------------------------------- Remplissage LCA -------------------------------------
# ---------------------------------------------------------------------------------------

# Remplissage LCA prenant en compte le zoom et le pan
def fill_polygon_LCA(canvas, points, color, useParity, scale, offset_x, offset_y):
    # Un polygone doit avoir au moins 3 sommets
    if len(points) < 3:
        return

    # Convertie les coordonnées monde en coordonnées écran
    scaled_points = [(x*scale + offset_x, y*scale + offset_y) for x, y in points]

    # Détermine les limites en y du polygone
    ymin = int(min(p[1] for p in scaled_points))
    ymax = int(max(p[1] for p in scaled_points))

    # ----------------------------- Construction de SI ( Structure d'Intersection )
    SI = {}  # dictionnaire [ymin] -> { ymax, xmin, inv }

    n = len(scaled_points)
    for i in range(n):
        # Pour chaque arête du polygone transformé
        x1, y1 = scaled_points[i]
        x2, y2 = scaled_points[(i+1) % n]

        # Si l'arête est horizontale, on l'ignore
        if y1 == y2: 
            continue

        if y1 < y2:
            ymin_e, ymax_e = int(y1), int(y2)
            x_at_ymin = x1
            inv_slope = (x2 - x1) / (y2 - y1)
            direction = 1 # haut
        else:
            ymin_e, ymax_e = int(y2), int(y1)
            x_at_ymin = x2
            inv_slope = (x1 - x2) / (y1 - y2)
            direction = -1 # bas

        SI.setdefault(ymin_e, []).append( {"ymax": ymax_e, "xmin": x_at_ymin, "inv": inv_slope, "dir": direction} )

    LCA = []

    # Pour chaque ligne de balayage
    for y in range(ymin, ymax):
        # ----------------------------- Création de LCA ( Liste des Côtes Actifs )

        # SI[y] contient toutes les arêtes commençant à la ligne de balayage actuelle
        # extend ajoute toutes ces arêtes à LCA
        if y in SI:
            LCA.extend(SI[y])

        # Retire les arêtes qui se termine à la ligne de balayage actuelle
        LCA = [e for e in LCA if e["ymax"] != y]

        # Trie les arêtes actives par ordre croissant des 'xmin'
        LCA.sort(key=lambda e: e["xmin"])

        drawLine = False 
        if useParity:
            # ----------------------------- Remplissage par la règle du pair-impair
            parity = False # en dehors du polygone
            for i in range(len(LCA) - 1):
                parity = not parity
                drawLine = parity
        else:
            # ----------------------------- Remplissage par la règle de l'enroulement 
            winding_number = 0
            for i in range(len(LCA) - 1):
                edge = LCA[i]
                winding_number += edge["dir"]
                drawLine = winding_number != 0
                    
        
        if drawLine:
            x_start = LCA[i]["xmin"]
            x_end = LCA[i+1]["xmin"]
            canvas.create_line(x_start, y, x_end, y, fill=color)

        # Mise à jour des x pour la prochaine itération
        for e in LCA:
            e["xmin"] += e["inv"]
    
'''
# Remplissage LCA prenant en compte le zoom et le pan 
def fill_polygon_LCA(canvas, points, color, scale, offset_x, offset_y): 
    # Un polygone doit avoir au moins 3 sommets 
    if len(points) < 3: 
        return 
        
    # Convertie les coordonnées monde en coordonnées écran 
    scaled_points = [(x*scale + offset_x, y*scale + offset_y) for x, y in points] 
    
    # Détermine les limites en y du polygone 
    ymin = int(min(p[1] for p in scaled_points)) 
    ymax = int(max(p[1] for p in scaled_points)) 
    
    edges = [] # arêtes non horizontales 
    n = len(scaled_points) 
    
    for i in range(n): 
        # Pour toutes les arêtes du polygone 
        x1, y1 = scaled_points[i] 
        x2, y2 = scaled_points[(i+1) % n] 
        
        # Si l'arête n'est pas horizontal 
        if y1 != y2: 
            # Pour chaque arête non-horiontal, on stocke 
            # y_min : coordonnée minimum en y de l'arête 
            # y_max : coordonnée maximum en y de l'arête 
            # x_at_ymin : coordonnée x à y_min 
            # inv_slope : delta_x / delta_y 
            if y1 < y2: 
                edges.append((y1, y2, x1, (x2-x1)/(y2-y1))) 
            else: 
                edges.append((y2, y1, x2, (x1-x2)/(y1-y2))) 
            
    for y in range(int(ymin), int(ymax)): 
        # Pour chaque arête retenue, on vérifie si la ligne de scan en y l'intersecte 
        # Si oui, rajouter l'intersection en x de ces dernières dans inter 
        inter = [] 
        
        for y1, y2, x_at_ymin, inv_slope in edges: 
            if y1 <= y and y < y2: 
                interX = x_at_ymin + (y - y1) * inv_slope 
                inter.append(interX) 
        
        if len(inter) == 0 : 
            continue # Il n'y a rien à dessiner 
        else: 
            # Trie les points d'intersection de gauche à droite et dessine une droite 
            inter.sort() 
            canvas.create_line(inter[0], y, inter[1], y, fill=color)
'''

# ---------------------------------------------------------------------------------------
# ------------------------- Structure de données d'un polygone --------------------------
# ---------------------------------------------------------------------------------------

class Polygon:
    def __init__(self, points, color="black"):
        self.points = points  # Liste de tuples (x, y)
        self.color = color # Couleur RBG

# ---------------------------------------------------------------------------------------
# --------------------------- Interface graphique avancée -------------------------------
# ---------------------------------------------------------------------------------------

class App:
    def __init__(self, root):
        self.root = root
        self.canvas = tk.Canvas(root, bg="white", width=900, height=700)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Listes de polygones et fenêtres
        self.polygons = []
        self.windows = []
        self.current_points = []

        # Modes et couleurs
        self.mode = "poly"
        self.poly_color = "blue"
        self.window_color = "green"
        self.fill_color = "orange"

        # Affichage de fenêtrage et remplissage
        self.show_clip = True
        self.show_fill = True
        self.useParity = True

        # Zoom / Pan
        self.scale = 1.0
        self.offset_x = 0
        self.offset_y = 0
        self.pan_start = None

        # Bindings souris
        self.canvas.bind("<Button-1>", self.add_point)                  # Clic droit
        self.canvas.bind("<Double-Button-1>", self.finish_polygon)      # Double clic droit
        self.canvas.bind("<Button-3>", self.show_menu)                  # Clic gauche
        self.canvas.bind("<Button-4>", self.zoom_mousewheel)            # roulement molette up ( Linux )
        self.canvas.bind("<Button-5>", self.zoom_mousewheel)            # roulement molette down ( Linux )
        self.canvas.bind("<Button-2>", self.start_pan)                  # Clic molette
        self.canvas.bind("<B2-Motion>", self.pan)                       # Mouvement souris

        # Menu avec couleurs, modes, reset
        self.menu = tk.Menu(root, tearoff=0)
        self.menu.add_command(label="Couleur polygone", command=self.choose_poly_color)
        self.menu.add_command(label="Couleur fenêtre", command=self.choose_window_color)
        self.menu.add_command(label="Couleur remplissage", command=self.choose_fill_color)
        self.menu.add_command(label="Changer de règle de remplissage", command=self.change_fill_rule)
        self.menu.add_separator()
        self.menu.add_command(label="Activer/Désactiver fenêtrage", command=self.toggle_clip)
        self.menu.add_command(label="Activer/Désactiver remplissage", command=self.toggle_fill)
        self.menu.add_separator()
        self.menu.add_command(label="Tracer polygone", command=lambda: self.set_mode("poly"))
        self.menu.add_command(label="Tracer fenêtre", command=lambda: self.set_mode("window"))
        self.menu.add_separator()
        self.menu.add_command(label="Tout réinitialiser", command=self.reset_all)

        # Label pour mode actif
        self.mode_label = tk.Label(root)
        self.set_title()
        self.mode_label.pack(side=tk.BOTTOM)

    # ---------------------------------------------------------------------------------------
    # ---------------------------------- Fonctions menu -------------------------------------
    # ---------------------------------------------------------------------------------------

    def show_menu(self, event):
        self.current_points = [] # peut annuler la saisie d'un polygone/fenêtre en ouvrant le menu
        self.redraw()
        self.menu.post(event.x_root, event.y_root)

    def choose_poly_color(self):
        c = colorchooser.askcolor(color=self.poly_color)[1]
        if c:
            self.poly_color = c
            self.redraw()  

    def choose_window_color(self):
        c = colorchooser.askcolor(color=self.window_color)[1]
        if c:
            self.window_color = c
            self.redraw()

    def choose_fill_color(self):
        c = colorchooser.askcolor(color=self.fill_color)[1]
        if c:
            self.fill_color = c
            self.redraw()

    def change_fill_rule(self):
        self.useParity = not self.useParity
        self.set_title()
        self.redraw()

    def toggle_clip(self):
        self.show_clip = not self.show_clip
        self.redraw()

    def toggle_fill(self):
        self.show_fill = not self.show_fill
        self.redraw()

    def reset_all(self):
        self.polygons.clear()
        self.windows.clear()
        self.current_points.clear()
        self.scale = 1.0
        self.offset_x = 0
        self.offset_y = 0
        self.redraw()

    # ---------------------------------------------------------------------------------------
    # ------------------------------- Dessin & Interaction ----------------------------------
    # ---------------------------------------------------------------------------------------

    def set_title(self):
        fillRuleTxt = "règle de parité" if self.useParity else "règle de l'enroulement"
        self.mode_label.config(text=f"Mode séléction: {self.mode}, Mode remplissage: {fillRuleTxt}")

    def set_mode(self, mode):
        self.current_points = []
        self.mode = mode
        self.set_title()

    def add_point(self, event):
        # coordonnées écran aux coordonnées monde
        x = (event.x - self.offset_x) / self.scale
        y = (event.y - self.offset_y) / self.scale
        self.current_points.append((x, y))
        if len(self.current_points) > 1:
            self.canvas.create_line(
                self.current_points[-2][0]*self.scale + self.offset_x,
                self.current_points[-2][1]*self.scale + self.offset_y,
                x*self.scale + self.offset_x,
                y*self.scale + self.offset_y
            )

    def finish_polygon(self, event=None):
        if len(self.current_points) < 3:
            self.current_points = []
            self.redraw()
            return

        if self.mode == "poly":
            self.polygons.append(Polygon(self.current_points.copy(), self.poly_color))
        elif self.mode == "window":
            if not is_counterClockWise(self.current_points):
                self.current_points.reverse()
            self.windows.append(Polygon(self.current_points.copy(), self.window_color))

        self.current_points = []
        self.redraw()

    def draw_polygon(self, points, color):
        for i in range(len(points)):
            x1, y1 = points[i]
            x2, y2 = points[(i+1) % len(points)]
            self.canvas.create_line(x1*self.scale + self.offset_x,
                                    y1*self.scale + self.offset_y,
                                    x2*self.scale + self.offset_x,
                                    y2*self.scale + self.offset_y,
                                    fill=color)

    def redraw(self):
        self.canvas.delete("all")
        
        for p in self.polygons:
            self.draw_polygon(p.points, self.poly_color)
        for w in self.windows:
            self.draw_polygon(w.points, self.window_color)
        
        if self.show_clip or self.show_fill:
            self.process_clip_fill()

    def process_clip_fill(self):
        for w in self.windows:
            triangles = earcut_triangulation(w.points)
            for poly in self.polygons:
                for tri in triangles:
                    clipped = sutherland_hodgman(poly.points, tri)
                    if len(clipped) >= 3:
                        if self.show_fill:
                            fill_polygon_LCA(self.canvas, clipped, self.fill_color, self.useParity, self.scale, self.offset_x, self.offset_y)
                        if self.show_clip:
                            self.draw_polygon(clipped, "red")

    # ---------------------------------------------------------------------------------------
    # ------------------------------------ Zoom & Pan ---------------------------------------
    # ---------------------------------------------------------------------------------------

    def zoom_mousewheel(self, event):
        factor = 1.0
        if event.num == 4:
            factor = 1.1
        elif event.num == 5:
            factor = 0.9

        # Zoom centré sur le centre de la souris
        mx, my = event.x, event.y
        self.offset_x = mx - (mx - self.offset_x) * factor
        self.offset_y = my - (my - self.offset_y) * factor
        self.scale *= factor
        self.redraw()

    def start_pan(self, event):
        self.pan_start = (event.x, event.y)

    def pan(self, event):
        dx = event.x - self.pan_start[0]
        dy = event.y - self.pan_start[1]
        self.offset_x += dx
        self.offset_y += dy
        self.pan_start = (event.x, event.y)
        self.redraw()

# ---------------------------------------------------------------------------------------
# ---------------------------- Lancement de l'application -------------------------------
# ---------------------------------------------------------------------------------------

if __name__ == "__main__":
    root = tk.Tk()
    root.title("Fenêtrage et remplissage")
    app = App(root)
    root.mainloop()