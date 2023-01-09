# my-secure-chat, c'est quoi ?

c'est un petit système de messagerie chiffrée que j'ai dévelloppé seul. là ou ça devient sympa, c'est que ce projet repose 
sur un concepte proche du p2p, c'est à dire qu'il n'y as pas de serveur centrale auquel tous les clients se connectent, mais
chaque client gère lui même l'envoie et le reception de ses messages de manière très ergonomique. Ce qui permet donc une meilleur
sécurité car les messgages sont chiffrée de bout en bout et qucun intermédiaire n'est utilisée !

## sommaire :

- [Installation](#Installation)
- [Utilisation](#Utilisation)
- [Toutes les commandes](#Toutes-les-commandes)
- [IP publique](#IP-publique)
- [Détail technique](#Détail-technique)

## Installation

dépandance à intaller :
|  nom      | utilité                                    |
|-----------|--------------------------------------------|
| uuid      | pour l'id des contactes                    |
| libsodium | très bonne lib pour chiffrer les messages  |
| pthread   | c'est sympas le multithreading non ?       |


Pour debian et distributions similaires:

    sudo apt install pthread uuid-dev libsodium 

ensuite il suffie d'utiliser le Makefile fournis :
   
    make all
    
puis :
   
    sudo make install
la selection du pseudo etc. s'effectuera lors première lancement.

## Utilisation
pour lancer l'app, il suffit de faire taper `my-secure-chat` dans un terminal.

l'application et en soit simple d'utilisation, on distingue 2 cas: 

- le cas où vous n'êtes pas en discution avec quelqu'un :  le texte rentrée est interprété comme une commandes qu'il dispose ou non du prefix `/`.
- le cas où vous êtes en discussion :  le texte intérpréter est envoyé à l'autre utilisateur sauf si le prefix `/` est emlpoyé, il est alors
traité comme une commande.

## Toutes les commandes
