# TP1 — Identification paramétrique d'un système mécanique linéaire

Ce dépôt contient l'ensemble des scripts MATLAB réalisés dans le cadre du TP1 portant sur l'identification des paramètres physiques (masses, raideurs, amortissements) d'un système mécanique à 3 degrés de liberté.

## Fichiers rendus

```text
TP1/
├── definit_param.m
├── simule_systeme.m
├── identifie_parametres.m
├── question1.m
├── question2.m
├── question3.m
├── question4.m
└── figure1.m
```

### 1. Définition et Simulation

#### `definit_param.m`
Ce script définit les paramètres nominaux utilisés pour la simulation du système mécanique :
* **Raideurs** : `k0 = 300`, `k1 = 100`, `k2 = 50` (en N/m)
* **Amortissements** : `b0 = 30`, `b1 = 40`, `b2 = 8` (en Ns/m)
* **Masses** : `m1 = 1`, `m2 = 1`, `m3 = 4` (en kg)

#### `simule_systeme.m`
Ce script utilise les paramètres définis pour construire les fonctions de transfert du système sous MATLAB. La variable de Laplace est créée avec `s = tf('s');`. Les polynômes intermédiaires `F1` à `F7` sont définis puis utilisés pour calculer les trois fonctions de transfert `G1`, `G2` et `G3`. Celles-ci correspondent aux réponses associées aux déplacements `alpha`, `beta` et `gamma`.
La simulation est effectuée sur une durée de 8 secondes avec un pas d'échantillonnage de 0,01 s (`t = 0:0.01:8`). Le script génère les positions, vitesses et accélérations idéales pour les trois masses.

### 2. Algorithme d'identification

#### `identifie_parametres.m`
C'est le cœur du programme d'identification. Ce script :
1. Construit la fonction génératrice de la matrice des régresseurs $\Phi(x)$ à partir des équations de la dynamique.
2. Sélectionne un ensemble de points temporels (indices d'échantillonnage 5, 10, 50, 100, 150, 200, 300, 500, 700) pour rendre le système surdéterminé.
3. Applique, selon les variables activées par les scripts appelants, une **quantification** (résolution finie des capteurs) et/ou un **filtrage** (`filtfilt`).
4. Résout le système linéaire $A x = Y$ par la méthode des moindres carrés (opérateur `\`) pour extraire le vecteur des paramètres identifiés.

### 3. Scripts d'analyse (Questions du TP)

Ces scripts orchestrent la simulation et l'identification pour mettre en évidence différents phénomènes physiques et numériques :

* **`question1.m`** : Identification sur mesures **idéales** (continues). Les paramètres identifiés correspondent exactement aux paramètres réels aux erreurs d'arrondi près.
* **`question2.m`** : Introduction de la **quantification** des capteurs ($10 \mu m$ pour la position, $0,1 mm/s$ pour la vitesse, $1 mm/s^2$ pour l'accélération). Met en évidence la perte de précision (notamment sur $m_1$).
* **`question3.m`** : Étude d'un **couplage fort** ($k_1 = 5000$, $b_1 = 400$) avec quantification. Démontre l'effondrement de l'identification lorsque la dynamique limite les mouvements relatifs entre les masses.
* **`question4.m`** : Ajout d'un **filtrage non déphasant** (passe-bas) via le paramètre `u=0.999` pour lisser les effets de la quantification et restaurer le conditionnement de la matrice d'identification.

### 4. Visualisation

* **`figure1.m`** : Génère une figure regroupant trois sous-graphes (subplots) illustrant l'évolution des positions, vitesses et accélérations mesurées (idéales) en fonction du temps.

## Exécution

Ouvrir MATLAB et se placer dans le dossier `TP1`.

Pour analyser le comportement du système étape par étape, exécutez simplement les scripts des questions dans la fenêtre de commande :

```matlab
question1
```
(Puis `question2`, `question3`, `question4` selon ce que vous souhaitez observer). 

Ces scripts appellent automatiquement `definit_param.m`, `simule_systeme.m` et `identifie_parametres.m`. Après exécution, les paramètres identifiés (`x`) et les pourcentages d'erreurs relatifs (`x_err`) s'affichent directement dans le Command Window.

Pour visualiser les signaux :
```matlab
figure1
```

## Prérequis
- MATLAB (Testé sur une version standard)
- Control System Toolbox (pour les commandes `tf`, `minreal`, `step`)
- Signal Processing Toolbox (pour la commande `filtfilt` utilisée dans la Q4)
