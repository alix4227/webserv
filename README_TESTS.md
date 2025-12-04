# Tests des Codes d'Erreur HTTP - Webserv

Ce dossier contient des scripts de test pour vérifier tous les codes d'erreur HTTP du serveur Webserv.

## Scripts de Test Disponibles

### 1. `test_error_codes.sh` (Bash)
Script bash complet qui teste tous les codes d'erreur avec curl.

**Avantages:**
- Pas de dépendances (utilise curl)
- Sortie colorée et détaillée
- Tests rapides

**Utilisation:**
```bash
./test_error_codes.sh
```

### 2. `test_error_codes.py` (Python)
Script Python utilisant la bibliothèque `requests` pour des tests plus avancés.

**Avantages:**
- Gestion des erreurs plus robuste
- Code plus lisible et maintenable
- Résumé détaillé des résultats

**Installation des dépendances:**
```bash
pip3 install requests
```

**Utilisation:**
```bash
./test_error_codes.py
# ou
python3 test_error_codes.py
```

### 3. `test_errors.sh` (Script existant)
Script bash original pour tester les pages d'erreur.

## Codes d'Erreur Testés

### Erreurs Client (4xx)

| Code | Description | Test |
|------|-------------|------|
| 403 | Forbidden | Path traversal (`../etc/passwd`) |
| 404 | Not Found | Fichiers inexistants, CGI inexistant |
| 405 | Method Not Allowed | Méthodes PATCH, PUT non autorisées |
| 413 | Payload Too Large | Upload fichier > 2MB |

### Erreurs Serveur (5xx)

| Code | Description | Test |
|------|-------------|------|
| 500 | Internal Server Error | CGI avec erreur de syntaxe |
| 502 | Bad Gateway | CGI qui crash ou timeout |
| 504 | Gateway Timeout | CGI qui prend trop de temps |

### Codes de Succès (2xx) - Contrôle

| Code | Description | Test |
|------|-------------|------|
| 200 | OK | Accès pages existantes |
| 201 | Created | Upload réussi |

## Prérequis

1. **Compiler et lancer le serveur:**
```bash
make
./webserv ./conf/webserv.conf
```

2. **Dans un autre terminal, lancer les tests:**
```bash
# Option 1: Script bash
./test_error_codes.sh

# Option 2: Script Python
./test_error_codes.py
```

## Structure des Répertoires Nécessaires

Assurez-vous que ces répertoires et fichiers existent:

```
Webserv/
├── www/
│   ├── index.html
│   ├── cgi-bin/          # Pour les tests CGI
│   └── upload/           # Pour les tests d'upload
├── error_pages/
│   ├── 403.html
│   ├── 404.html
│   ├── 405.html
│   ├── 413.html
│   ├── 500.html
│   └── 502.html
└── conf/
    └── webserv.conf
```

## Interprétation des Résultats

### ✓ PASSED (Vert)
Le test a réussi, le code HTTP reçu correspond au code attendu.

### ✗ FAILED (Rouge)
Le test a échoué, le code HTTP reçu ne correspond pas au code attendu.

### ⚠ SKIPPED (Jaune)
Le test a été sauté (fichier ou répertoire manquant).

## Configuration

Par défaut, les tests utilisent:
- **Host:** localhost
- **Port:** 8080

Pour modifier ces valeurs, éditez les variables en haut des scripts:

**Dans `test_error_codes.sh`:**
```bash
PORT=8080
HOST="localhost:$PORT"
```

**Dans `test_error_codes.py`:**
```python
HOST = "localhost"
PORT = 8080
```

## Vérifications Importantes

Les tests vérifient:
1. ✅ Le **code HTTP** correspond au code attendu
2. ✅ Le **Content-Type** est `text/html` pour les pages d'erreur
3. ✅ Le **contenu** de la réponse est valide
4. ✅ Les pages d'erreur personnalisées sont servies

## Dépannage

### "Le serveur n'est pas accessible"
- Vérifiez que le serveur est démarré
- Vérifiez le port (8080 par défaut)
- Vérifiez qu'aucun autre service n'utilise le port

### "Impossible de créer le script de test"
- Vérifiez les permissions sur `./www/cgi-bin/`
- Créez le répertoire: `mkdir -p ./www/cgi-bin`

### Tests CGI qui échouent
- Vérifiez que Python3 est installé: `python3 --version`
- Vérifiez le chemin dans `webserv.conf`: `cgi .py /usr/bin/python3`
- Vérifiez les permissions d'exécution des scripts

## Exemples de Sortie

### Test Réussi
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Test: 404 - Fichier inexistant
URL: http://localhost:8080/fichier_inexistant.html
Méthode: GET
Code attendu: 404
Code reçu: 404
Content-Type: text/html
✓ PASSED - Code HTTP correct
```

### Test Échoué
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Test: 413 - Body trop volumineux
URL: http://localhost:8080/upload
Méthode: POST
Code attendu: 413
Code reçu: 200
✗ FAILED - Code HTTP incorrect (attendu: 413, reçu: 200)
```

## Tests Manuels dans le Navigateur

Vous pouvez également tester manuellement en ouvrant ces URLs:

- **404:** http://localhost:8080/inexistant.html
- **403:** http://localhost:8080/../etc/passwd
- **405:** Utilisez un outil comme Postman pour envoyer une requête PATCH

## Contribution

Pour ajouter un nouveau test:

1. Ajoutez un nouveau `test_request()` dans le script
2. Incrémentez les compteurs de tests
3. Documentez le test dans ce README

## Notes

- Les tests CGI créent temporairement des scripts dans `./www/cgi-bin/` et les suppriment après
- Les tests d'upload créent des fichiers temporaires dans `/tmp/`
- Tous les fichiers temporaires sont nettoyés à la fin des tests
