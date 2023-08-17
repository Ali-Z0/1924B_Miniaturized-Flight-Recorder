// Mc32Debounce.c

// C. HUBER    19/02/2014 Portage sur PIC32MX
//             Attention int1 transformé en uint8
//    Modifications :
//         utilisation stdint et stdbool  10.02.2015 CHR
//
// Fonctions pour la gestion de l'anti-rebond simplifiée d'un switch
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

#include "Mc32Debounce.h"

// constante comptage stabilité de l'anti-rebond
const uint8_t MaxDebounceCount = 5;

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

void DoDebounce (S_SwitchDescriptor *Descriptor, bool InputValue)
{
   bool PrevInputValue;
   s_bits tmp;
   
   tmp = Descriptor->bits;
    
   
   // Traitement selon état du mécanisme
   switch (Descriptor->DebounceState)
   {
      case DebounceWaitChange :
      PrevInputValue = tmp.KeyPrevInputValue;
         if ( ! (InputValue == PrevInputValue) ) {
               tmp.KeyPrevInputValue = InputValue;
               Descriptor->DebounceState = DebounceWaitStable;
          }
      break;

      case DebounceWaitStable :
         PrevInputValue = tmp.KeyPrevInputValue;
         if ( InputValue == PrevInputValue )
         {
              Descriptor->DebounceCounter++;
              if (Descriptor->DebounceCounter >= MaxDebounceCount)  {
                  // mise à jour du nouvel état
                  tmp.KeyValue = InputValue;
                  if (tmp.KeyValue == 0) {
                     tmp.KeyPressed = 1;
                  } else {
                     tmp.KeyReleased = 1;
                  }
                  Descriptor->DebounceState = DebounceWaitChange;
                  Descriptor->DebounceCounter = 0;
               }
          } else {
            Descriptor->DebounceCounter = 0;
            tmp.KeyPrevInputValue = InputValue;
         }
      break;
   }
   Descriptor->bits = tmp;
 } // end DoDebounce

/********************************************************************************************/
// DebounceInit (S_SwitchDescriptor *pDescriptor) :
//              Routine effectuant l'initialisation de la structure d'un switch
//              dont l'adresse de la structure est passée en paramètre.
//
// Entrée: un pointeur sur la structure du switch
//

void DebounceInit (S_SwitchDescriptor *pDescriptor) {
   pDescriptor->DebounceState     = DebounceWaitChange;   // état de l'antirebond
   pDescriptor->DebounceCounter   = 0;
   pDescriptor->bits.KeyPressed        = 0;
   pDescriptor->bits.KeyReleased       = 0;
   pDescriptor->bits.KeyPrevInputValue = 1;
   pDescriptor->bits.KeyValue          = 1;
}

//  DebounceGetInput  fourni l'état du switch aprés anti-rebond
bool DebounceGetInput (S_SwitchDescriptor *pDescriptor) {
   return (pDescriptor->bits.KeyValue);
}

//  DebounceIsPressed    true indique que l'on vient de presser la touche
bool DebounceIsPressed (S_SwitchDescriptor *pDescriptor) {
   return (pDescriptor->bits.KeyPressed);
}

//  DebounceIsReleased   true indique que l'on vient de relacher la touche
bool DebounceIsReleased (S_SwitchDescriptor *pDescriptor) {
   return (pDescriptor->bits.KeyReleased);
}

//  DebounceClearPressed    annule indication de pression sur la touche
void DebounceClearPressed  (S_SwitchDescriptor *pDescriptor) {
   pDescriptor->bits.KeyPressed = 0;
}

//  DebounceClearReleased   annule indication de relachement de la touche
void DebounceClearReleased  (S_SwitchDescriptor *pDescriptor) {
   pDescriptor->bits.KeyReleased = 0;
}
