#include <stdio.h>

// On définit les symboles pour éviter de manipuler des chiffres flous
typedef enum {
    PIPE = 0,     // 5 au total
    AMPOULE,      // 5
    POING,        // 5
    COURONNE,     // 5
    LIVRE,        // 4
    COLLIER,      // 3
    OEIL,         // 3
    CRANE         // 3
} Symbole;

// Structure pour un suspect
typedef struct {
    char nom[30];
    int attributs[8]; // Tableau de 0 ou 1 pour chaque symbole
} Suspect;