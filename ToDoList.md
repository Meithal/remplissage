To do List :
===

Menu
---

Options :

- [x] Générer/Arrêter de générer un polygone : placer des points d'une couleur choisie et les connecte avec une ligne. Quand on arrête la génération, le premier et dernier point sont connectés.
- [x] Choisir une couleur : déjà implémenté
- [x] Générer/Arrêter de générer une fenêtre : même comportement que Générer/Arrêter de générer un polygone 
- [x] Activer/Désactiver fenêtrage 
- [ ] Mode modification de position de point : cliquer sur l'écran ne génére pas des points mais modifie leur position en les draggant 
- [ ] Activer/Désactiver remplissage

Classes à définir
---

- [ ] Point.cpp/hpp -> définit un point et ses opérations
- [ ] Polygone.cpp/hpp -> définit un polygone
- [ ] Fenetre.cpp/hpp -> définit une fenetre
- [ ] Texture.cpp/hpp -> définit la texture affiché et ses opération et mises à jour
- [ ] Decoupage.cpp/hpp
- [ ] Remplissage.cpp/hpp

Découpage
---

- [ ] Implémenter Sutherland-hogdmann

Remplissage
---

- [ ] Implémenter LCA
- [ ] Implémenter les algos pour prendre en compte plusieurs polygones

Bonus
---

- [ ] Marche de Jarvis pour enrouler un nuage de points.

### Pour le fenêtrage :

- [ ] Implémenter le découpage par une fenêtre quelconque concave. Utiliser la méthode du ear cutting
(décomposer la fenêtre concave en plusieurs polygones convexes ou découpage en triangles).
- [ ] Proposer la gestion d'un zoom. Éventuellement effectuer un lissage de la partie agrandie
- [ ] Vous êtes libres de rajouter d'autres éléments apportant une touche personnelle au TP

### Pour le remplissage :

- [ ] Proposer deux types de remplissage pour les polygones croisé s: une qui compte le nombre
d'intersections, l'autre le nombre d'enroulement non nul.
- [ ] Possibilité de remplir uniquement une partie du polygone. D'abord il faudra le subdiviser, puis remplir
telle ou telle partie (idée: dans un premier temps, le décomposer en triangles)
- [ ] Vous êtes libres de rajouter d'autres éléments apportant une touche personnelle au TP

Taches
---

- [x] Quand on commence a dessiner un polygone, faire apparaitre
une fenetre pour mettre un terme au tracé.
- [x] Ne pas modifier l'etat interne des formes qu'on dessine a partir de gui.c
- [x] Faire qu'on puisse changer individuellemeent la couleur de chaque polyygone
- [ ] Mettre l'option de fenetrer soit sutherland-cohen, soit sutherland-hogmann
- [ ] Faire que le découpage Cohen marche dans les deux sens
- [x] Afficher sur l'UI la shape en cours.
- [x] Pouvoir modifier la shape en cours.
- [ ] Tester Sutherland Cohen en utilisant une texture masque et en appliquant
une soustraction dessous. Ca devrait pouvoir faire l'économie d'utiliser des layers.

Taches bonus
---

- [ ] Pouvoir cliquer sur un polygone pour en résumer le tracé.
- [x] Tester apres avoir résumé un tracé qu'on puisse en tracer
un nouveau.
- [ ] Pouvoir modifier individuellement l'épaisseur du trait
de chaque polygone.
- [ ] Migrer toute la logique de clic et drag hors de GUI et utiliser
fonctions GLFW natives.
- [ ] Pouvoir déplacer les vertices individuellement.
