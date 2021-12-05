Ce projet a été réalisé par Tafsir Mbodj NDIOUR (https://github.com/risfat97/messagerie-instantanee)

** Important: Vous devez disposer d'un système d'exploitation de la famille linux. **

## Note

Deux programmes clients lancés sur le même PC que le serveur ne peuvent pas communiquer.
Les deux programmes clients doivent être lancés sur deux PC différents.

## Commande valable 

A la racine du projet, vous pouvez lancer la commande:

### `make`

Compile les programmes et créé les exécutables `chat_server` et `chat`.

## Exécution 

Pour une bonne exécution des programmes vous devez toujours lancer d'abord le programme serveur avec la commande:

### `./chat_server`

Lance le serveur et attend de nouvelles connexions jusqu'à 5 maximum.

Une fois le serveur lancé vous pouvez lancer le programme client.

### `./chat <ip_serveur> <ip_de_votre_machine>`

Pour exécuter le programme client il faut donner en arguments l'adresse IP du serveur et l'adresse IP de votre machine.

