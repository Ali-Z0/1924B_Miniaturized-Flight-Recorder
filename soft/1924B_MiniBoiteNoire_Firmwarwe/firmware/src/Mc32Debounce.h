#ifndef MC32DEBOUNCE2_H
#define MC32DEBOUNCE2_H


// Mc32Debounce.h

// C. HUBER    19/02/2014 Portage sur PIC32MX
//             Attention int1 transformé en uint8

// Fonctions pour la gestion de l'anti-rebond d'un switch
//
//
// Principe : Il est nécessaire d'appeler cycliquement la fonction DoDebounce
//            pour chaque switch. Suggestion cycle de 1 ms
//
//            Pour la gestion du switch, il y a 3 fonctions à disposition :
//       DebounceGetInput  fourni l'état du switch aprés anti-rebond
//       DebounceIsPressed    true indique que l'on vient de presser la touche
//       DebounceIsReleased   true indique que l'on vient de relacher la touche
//       DebounceClearPressed    annule indication de pression sur la touche
//       DebounceClearReleased   annule indication de relachement de la touche
//
//  Remarques : toute les fonctions utilisent un descripteur de touche
//              un état 0 indique une touche pressée
//
//---------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

// définitions des types qui seront utilisés dans cette application

// Etats du système antirebond
typedef enum {
      DebounceWaitChange,
      DebounceWaitStable,
} E_DebouceStates;

// New
typedef struct {
   uint8_t  KeyPressed :1;              // événement touche pressée
   uint8_t  KeyReleased : 1;             // événement touche relachée
   uint8_t  KeyPrevInputValue : 1;       // valeur précédente de la touche
   uint8_t  KeyValue : 1;                // valeur finale (image) de la touche
 } s_bits;

// structure du descripteur
typedef struct {
           E_DebouceStates DebounceState;    // état de l'antirebond
           uint8_t     DebounceCounter;         // compteur
           s_bits bits;
} S_SwitchDescriptor;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// POUR CHAQUE SWITCH DONT ON VEUT EFFECTUER L'ANTI-REBOND DANS L'APPLICATION.
// IL FAUDRA DEFINIR UNE VARIABLE DU  TYPE S_SwitchDescriptor
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//
// DoDebounce (S_SwitchDescriptor *Descriptor, boolean InputValue)
//              Routine effectuant la gestion de l'anti-rebond d'un switch
//              dont l'adresse de la structure est passée en paramètre.
//              La valeur du switch lue sur le port est a passer lors de l'appel
//
// Entrée: un pointeur sur la structure du switch
//         la valeur du switch lue sur le port
//
// Sortie: la structure mise à jour
//         le bit KeyValue qui donne la valeur du switch après anti-rebond
//         le bit KeyPressed qui est mis à 1 dès que l'on presse sur le switch
//           ce bit n'est pas resetté par cette fonction mais doit l'être par l'application
//           si elle l'utilise.
//         le bit KeyReleased qui est mis à 1 dès que l'on relâche sur le switch
//           ce bit n'est pas resetté par cette fonction mais doit l'être par l'application
//           si elle l'utilise
//

void DoDebounce (S_SwitchDescriptor *Descriptor, bool InputValue);


/********************************************************************************************/
// DebounceInit (S_SwitchDescriptor *pDescriptor) :
//              Routine effectuant l'initialisation de la structure d'un switch
//              dont l'adresse de la structure est passée en paramètre.
//
// Entrée: un pointeur sur la structure du switch
//

void DebounceInit (S_SwitchDescriptor *pDescriptor);

//  DebounceGetInput  fourni l'état du switch aprés anti-rebond
bool DebounceGetInput (S_SwitchDescriptor *pDescriptor);

//  DebounceIsPressed    true indique que l'on vient de presser la touche
bool DebounceIsPressed (S_SwitchDescriptor *pDescriptor);

//  DebounceIsReleased   true indique que l'on vient de relacher la touche
bool DebounceIsReleased (S_SwitchDescriptor *pDescriptor);

//  DebounceClearPressed    annule indication de pression sur la touche
void DebounceClearPressed  (S_SwitchDescriptor *pDescriptor);

//  DebounceClearReleased   annule indication de relachement de la touche
void DebounceClearReleased  (S_SwitchDescriptor *pDescriptor);

#endif