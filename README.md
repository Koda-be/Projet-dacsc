# Projet DACSC

## Compiler l'étape 1

Installer GCC/G++, CMake, Qt 6 Widgets et les fichiers de développement du
connecteur MySQL ou MariaDB. Depuis la racine, `make -j2 DACSC` construit le
serveur, le client Qt et les exécutables de test. `make check` exécute le test
de transport OBEP sans base de données.

Le Makefile cherche le connecteur via `pkg-config`, puis `mysql_config` ou
`mariadb_config`. Si nécessaire, indiquer les options d'en-tête et de lien
avec `MYSQL_CFLAGS` et `MYSQL_LIBS`.

## Configurer et lancer

Depuis la racine, créer les fichiers locaux à partir des exemples.

```sh
umask 077
cp -i .client.cfg.example .client.cfg
cp -i .server.cfg.example .server.cfg
cp -i .mysql-dacsc.cnf.example .mysql-dacsc.cnf
```

- `.client.cfg` contient l'adresse ou le nom d'hôte suivi du port TCP.
- `.server.cfg` contient `PORT_ENCODING`, `NTHREAD` (1 à 128) et, si désiré,
  `BIND_ADDRESS`. Les séparateurs `=` et `:` sont acceptés. Quand
  `BIND_ADDRESS` est absent, le serveur écoute sur toutes les interfaces IPv4,
  comme auparavant. L'exemple écoute seulement sur la boucle locale. OBEP
  transmet les identifiants applicatifs en clair.
- `.mysql-dacsc.cnf` est requis. Renseigner le compte local dans le groupe
  `[dacsc]`, conserver les permissions `0600` et adapter l'hôte, le port ou
  la socket au besoin. Définir `database=PourStudent` ou `database=DACSC`
  selon le schéma existant sur la machine. Conserver une seule ligne
  `database` dans le groupe. Le serveur s'arrête au démarrage si le schéma
  est absent, introuvable ou inaccessible au compte configuré.
  Le compte doit pouvoir lire les employés et lire et ajouter les auteurs,
  sujets et livres. Les identifiants MySQL et ceux utilisés dans Qt sont
  distincts.

Lancer `./bin/release/server` et `./bin/release/ClientBookEncoderQt` dans
deux terminaux, chacun depuis la racine. Les fichiers locaux sont ignorés
par Git. Sur une installation existante, ajouter la ligne `database` au
fichier d'options MySQL local avant de redémarrer le serveur.

## Tester avec une base existante

Serveur démarré, exécuter `python3 tests/stage1_integration.py 127.0.0.1 45678 .test-dacsc.credentials`.
Le fichier local contient le login et le mot de passe d'un compte applicatif
existant, sur deux lignes. L'essai vérifie l'authentification, les listes,
LOGOUT et les reconnexions. L'option `--insert` vérifie aussi les ajouts, les
IDs et les doublons, et laisse trois lignes de test dans la base. Le script
n'initialise ni le schéma ni le compte.
