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

Bonus :

- [ ] Marche de Jarvis pour enrouler un nuage de points.
- [ ] Implémenter le découpage par une fenêtre quelconque concave. Utiliser la méthode du ear cutting
(décomposer la fenêtre concave en plusieurs polygones convexes ou découpage en triangles).
- [ ] Proposer la gestion d'un zoom. Éventuellement effectuer un lissage de la partie agrandie
- [ ] Vous êtes libres de rajouter d'autres éléments apportant une touche personnelle au TP

Remplissage
---

- [ ] Implémenter LCA
- [ ] Implémenter les algos pour prendre en compte plusieurs polygones

Bonus

- [ ] Proposer deux types de remplissage pour les polygones croisé s: une qui compte le nombre
d'intersections, l'autre le nombre d'enroulement non nul.
- [ ] Possibilité de remplir uniquement une partie du polygone. D'abord il faudra le subdiviser, puis remplir
telle ou telle partie (idée: dans un premier temps, le décomposer en triangles)
- [ ] Vous êtes libres de rajouter d'autres éléments apportant une touche personnelle au TP

Bezier
---

- [ ] Cliquer n+1 points de contrôle à l'écran et afficher le polygone de contrôle
- [ ] Afin d'engendrer la courbe de Bézier d'ordre n associé à ce nuage de points, faire deux méthodes:
  - [ ] utiliser la formule directe avec le calcul des combinaisons à l'aide du triangle de Pascal (vu en
complément de cours)
  - [ ] utiliser l'algorithme de Casteljau, celui du cours en version itérative
Dans les deux cas, tracer la courbe à l'écran avec un pas fixé et comparer les temps de calculs pour un
nombre important de points (> 50)
Remarque importante: à l'aide de segments, il faudra relier 2 à 2 tous les points consécutifs de la courbe
de Bézier.
- [ ] Possibilité de modifier le pas, avec les touches + et - afin de pouvoir (ou non) lisser la courbe
- [ ] Pouvoir engendrer un nombre illimité de courbes, pour cela utiliser une liste chaînée
- [ ] Pouvoir parcourir la liste et supprimer n'importe quelle courbe
- [ ] Pouvoir déplacer et supprimer un ou plusieurs points de contrôle et afficher en temps réel la courbe
- [ ] Modifier les courbes: utiliser les matrices de translation, scaling, rotation et cisaillement sur les points de contrôle d'une Bézier et tracer en temps réel la courbe en fonction des points de contrôle modifiés.
  - [ ] Effectuer une gestion à la souris, éventuellement clavier.
- [ ] Effet de la multiplicité d'un point de contrôle: répéter un point de contrôle dans la liste des points de contrôle et vérifier que la courbe se rapproche de ce point
- [ ] Effectuer des découpages et remplissages, via les algo vus au T1, de courbes de Bézier fermées (le premier point de contrôle est confondu avec le dernier + éventuellement faire du raccordement C0, C1 et C2 pour fermer la courbe sans point anguleux)
- [ ] Raccorder des Courbes de Bézier, faire les raccords C0, C1 et C2. Vérifier le contrôle local pour unecourbe de Bézier constituée de 8 courbes de Bézier cubiques raccordées C2
- [ ] Bonus : Implémenter une méthode pour calculer l'enveloppe convexe d'un nuage de points (ex: méthode de Jarvis). Calculer les enveloppes convexes des lignes polygonales de contrôle de 2 Bézier. Tester si les deux enveloppes s'interceptent. Si c'est le cas calculer le point d'intersection éventuel entre les 2 Bézier

Partie 2

- [ ] Utiliser l'algorithme de Cox-de-Boor pour le calcul des bases Bsplines Bi,p(t) par récurrence, avec `0<=i<=n` et `t0<=t<=tn`. Attention pour le calcul des courbes Bsplines utiliser `tp<=t<=tn-p`. L'utilisateur devra:
  - [ ] Saisir le degré p
  - [ ] Cliquer les n-p points de contrôle
  - [ ] Le vecteur nodal (t0,...tn) sera par défaut uniforme, mais il faudra prévoir une éventuelle saisie clavier.
- [ ] Tester le cas d'une courbes BSpline cubique avec un vecteur nodal uniforme et non-uniforme.
- [ ] Faire des courbes BSplines interpolant aussi les points de contrôle extrémités
- [ ] Nombre quelconque de tracés de courbes
- [ ] Interpolation d'un point de contrôle quelconque. Gérer pour cela la répétition de nœuds ou de points de contrôle. Attention aux divisions par 0. Affecter 0 si cela se pré sente dans le calcul des Bi,p(t) correspondant.
- [ ] Vérifier le contrôle local pour une courbe BSpine cubique sur 20 points de contrôle.

Bonus:
- [ ] Pour le contrôle local des courbes, lors du déplacement d'un point de contrôle, re-calculer que les p+1 parties modifiées en utilisant que les points de contrôle et Bi,p(t) concernés.
- [ ] Engendrer les NURBS (classe mère de toute courbe), pour cela utiliser des poids sur les courbes BSplines
- [ ] Faire des tests
- [ ] Tracer des cercles, ellipses, paraboles, hyperboles


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
