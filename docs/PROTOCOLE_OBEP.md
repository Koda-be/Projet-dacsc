# Protocole OBEP — accord du binôme

*Synthèse de l'appel du 25 octobre 2026.*

## Décisions retenues

- **Serveur de connexions**, en C, avec un pool de threads POSIX.
- **Architecture opaque** pour le module OBEP.
- **En-tête fixe de 12 octets** : 4 caractères hexadécimaux pour la longueur du contenu, puis `|`, puis 7 octets pour le raccourci de commande.
- Le raccourci reprend le nom de commande sans underscores, limité aux 7 premiers caractères et complété avec des `0` s'il est plus court.
- Le contenu suit directement l'en-tête, sans séparateur supplémentaire, et se termine par un **octet nul `\0`**. La longueur annoncée comprend ce nul final, mais **pas l'en-tête**.

```text
[longueur : 4][|][commande : 7][contenu terminé par \0]
```

Les crochets ne sont pas transmis. La taille totale vaut `12 + longueur du contenu` octets.

- **Seul LOGIN est autorisé avant authentification.** LOGOUT retire l'autorisation d'exécuter les autres commandes jusqu'à un nouveau login réussi.
- **Trame invalide** : annuler la modification en cours et afficher une boîte d'information signalant l'erreur.
- Les séparateurs des données seront choisis parmi les caractères fournis par le professeur, absents des données ; aucun mécanisme d'échappement n'est prévu à ce stade.

## À préciser au fil du développement

Comme convenu, les formats exacts des paramètres et des réponses seront communiqués entre nous lors de l'implémentation, en respectant les données et résultats attendus aux pages 5–6 du syllabus :

`LOGIN`, `LOGOUT`, `GET_AUTHORS`, `GET_SUBJECTS`, `ADD_AUTHOR`, `ADD_SUBJECT`, `ADD_BOOK`.

Il reste notamment à expliciter le complément de `0` pour les commandes courtes, les séparateurs, l'encodage des textes, le format des listes/erreurs et le maintien ou la fermeture de la connexion après LOGOUT. Les conventions précisées seront ajoutées ici.

## Suite proposée jusqu'à l'évaluation 1

*Proposition de progression, à adapter ensemble ; ce n'est pas un nouvel accord de protocole.*

1. **Aligner les interfaces et les trames** : vérifier que les deux côtés appliquent les décisions ci-dessus, puis préciser les formats nécessaires à LOGIN/LOGOUT.
2. **Avancer en parallèle** sur le serveur de connexions (pool, attribution des connexions, port et nombre de threads lus dans un fichier de configuration) et sur les traitements OBEP avec accès MySQL côté serveur.
3. **Faire fonctionner LOGIN puis LOGOUT de bout en bout** : bouton Qt → bibliothèque de sockets → serveur → vérification en base → retour affiché. Vérifier aussi le refus des commandes sans authentification.
4. **Compléter les fonctions d'encodage** : charger auteurs/sujets après login, ajouter un auteur ou un sujet et actualiser les listes, puis ajouter un livre et l'afficher dans le tableau de la session.
5. **Valider l'ensemble avant l'évaluation** : plusieurs clients simultanés, échecs de login, doublons, références inexistantes, trames invalides ou reçues en plusieurs morceaux, et déconnexions. Vérifier la compilation et le lancement depuis les instructions du dépôt.
