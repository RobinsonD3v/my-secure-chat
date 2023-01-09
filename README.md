# my-secure-chat, c'est quoi ?

C'est un petit système de messagerie chiffrée que j'ai développé seul. Là où ça devient sympa, c'est que ce projet repose sur un concept proche du p2p, c'est à dire qu'il n'y a pas de serveur central auquel tous les clients se connectent, mais chaque client gère lui-même l'envoie et la réception de ses messages (de manière très ergonomique). Ce qui permet donc une meilleure sécurité et confidentialité car les messages sont chiffrées de bout en bout et qu'aucun intermédiaire n'est utilisée.

## sommaire :

- [Installation](#Installation)
- [Utilisation](#Utilisation)
- [Toutes les commandes](#Toutes-les-commandes)
- [IP publique](#IP-publique)
- [Détails techniques](#Détails-techniques)
- [Améliorations futured](#Améliorations-futures)

## Installation

dépandance à intaller :
|  nom      | utilité                                    |
|-----------|--------------------------------------------|
| uuid      | pour l'id des contactes                    |
| libsodium | très bonne lib pour chiffrer les messages  |
| pthread   | c'est sympas le multithreading non ?       |


Pour debian et distributions similaires:

    sudo apt install pthread uuid-dev libsodium 

ensuite, il suffit d'utiliser le Makefile que j'ai codé :
   
    make all
    
puis :
   
    sudo make install
**note:** la sélection du pseudo, etc. s'effectuera lors du premier lancement..

## Utilisation
pour lancer l'app, il suffit de taper `my_secure_chat` dans un terminal.

l'application est en soit simple d'utilisation, on distingue 2 cas: 

- le cas où tu n'es pas en discutions avec quelqu'un :  le texte rentré est interprété comme une commande, qu'il dispose ou non du préfixe `/`.
- le cas où tu es en discussions :  le texte interprété est envoyé à l'autre utilisateur sauf si le préfixe `/` est employé, il est alors traité comme une commande.

## Toutes les commandes

|  nom             | ce que ça fait                                 |
|------------------|------------------------------------------------|
| msg  [pseudo]    | créer/rejoin une conversation                  |
| new  [pseudo]    | identique à précédement                        |
| join [pseudo]    | identique à précédement                        |
| add              | ajoute un nouvel utilisateur                   |
| remove \<id\>    | supprime un utilisateur des contactes          |
| info \<pseudo\>  | affiche les infos du contacte (id,ip...)       |
| me               | affiche tes infos                              |
| modify \<id\>    | modifie les infos d'un contacte                |
| modify_me        | modifie tes informations                       |
| clear_db         | nettoie la base de données avec tes contactes  |
| clear_history    | supprime l'historique de conversation          |
| contactes        | affiche la base de données avec tes contactes  |
| online           | affiche les utilisateurs en ligne              |
| set_presence     | modifie ton status  (en ligne /hors ligne)     |
| get_presence     | permet de savoir qui est en ligne              |
| send_contact     | partage aléatoirement 10 contactes             |
| exit             | quitte l'application ou la conversation        |
| clear            | efface l'écran                                 |
| help [commandes] | affiche le message d'aide                      |
| key              | génère une nouvelle clée privée                |

**note:** [arg] designe un argument facultatif alors que \<arg\> est un argument obligatoire

## IP publique

Pour pouvoir communiquer avec des personnes en dehors de ton réseau privé, il faut utiliser ton ip publique et ne pas oublier de rediriger le port 8080 de ton routeur vers ton ordinateur.

## Détails techniques

de manière plus détaillé voici comment fonction ce petit projet :

2 fonctions sont lancées en parallèles, une gère l'envoie de message, l'autre la réception.
Celle qui envoie les messages s'occupe aussi du parsing des commandes, du chiffrement des messages ainsi que de leurs affichages (donc déchiffrement). Alors que la fonction de réception, elle ne gère que la création de l'historique.

Pour le chiffremment, j'utilise libsodium et je procède comme ceci :
- création d'un jeu de clée publique/privée
- envoie d'une clée privée (chiffré)
- utilisation de cette clée pour déchiffrer les messages

Clairement pas les plus simple et efficace mais c'est fun et je peux utiliser un algo de chiffrement très sûr, rapide et pratique (chacha20-poly1305)

**note:** il est assez évident que ce programme ne fonctionne pas sous win10 et cie, ce qui n'a aucun intêret sachant qu'il recherche sécurité et confidentialité. 

## Amélioration future

Si jamais des personnes étaient intérréssées par ce projet, je pourrais tout à fait le rendre beaucoup plus propre et le continuer quelques pistes d'amméliorations sont :

À court termes :
- un vrai affichage (pas bricollé quoi)
- un envoie de fichier beaucoup plus rapide (par blocs)
- chiffrer l'envoie de fichier

À long termes :
- rendre le code plus lisible
- une vrai documentation
- puis sans doute pleins d'autres choses
