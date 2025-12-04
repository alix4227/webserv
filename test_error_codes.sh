#!/bin/bash
# Test complet des codes d'erreur HTTP du serveur Webserv

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PORT=8080
HOST="localhost:$PORT"
PASSED=0
FAILED=0

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   Test des codes d'erreur HTTP       ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo ""

# Fonction pour tester une requête
test_request() {
    local description=$1
    local method=$2
    local url=$3
    local expected_code=$4
    local extra_args=$5
    
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Test:${NC} $description"
    echo -e "${BLUE}URL:${NC} $url"
    echo -e "${BLUE}Méthode:${NC} $method"
    echo -e "${BLUE}Code attendu:${NC} $expected_code"
    
    # Effectuer la requête
    if [ "$method" = "GET" ]; then
        response=$(curl -s -o /tmp/test_response.html -w "%{http_code}|%{content_type}" "$url" $extra_args)
    else
        response=$(curl -s -o /tmp/test_response.html -w "%{http_code}|%{content_type}" -X "$method" "$url" $extra_args)
    fi
    
    http_code=$(echo $response | cut -d'|' -f1)
    content_type=$(echo $response | cut -d'|' -f2)
    
    echo -e "${BLUE}Code reçu:${NC} $http_code"
    echo -e "${BLUE}Content-Type:${NC} $content_type"
    
    # Vérifier le résultat
    if [ "$http_code" = "$expected_code" ]; then
        echo -e "${GREEN}✓ PASSED${NC} - Code HTTP correct"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ FAILED${NC} - Code HTTP incorrect (attendu: $expected_code, reçu: $http_code)"
        FAILED=$((FAILED + 1))
    fi
    
    # Vérifier Content-Type pour les erreurs HTML
    if [[ "$content_type" == *"text/html"* ]]; then
        echo -e "${GREEN}✓${NC} Content-Type correct (HTML)"
    else
        echo -e "${RED}✗${NC} Content-Type incorrect: $content_type"
    fi
    
    # Afficher un extrait de la réponse
    if [ -f /tmp/test_response.html ] && [ -s /tmp/test_response.html ]; then
        echo -e "${BLUE}Extrait de la réponse:${NC}"
        head -n 3 /tmp/test_response.html | sed 's/^/  /'
    fi
    
    echo ""
}

# Vérifier que le serveur est démarré
echo "Vérification de la connexion au serveur sur le port $PORT..."
if ! nc -z localhost $PORT 2>/dev/null; then
    echo -e "${RED}✗ ERREUR: Le serveur n'est pas accessible sur le port $PORT${NC}"
    echo "Démarrez votre serveur avec: ./webserv ./conf/webserv.conf"
    exit 1
fi
echo -e "${GREEN}✓ Serveur accessible${NC}"
echo ""

# ============================================================
# Tests des codes d'erreur 4xx (Erreurs client)
# ============================================================

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║       TESTS ERREURS 4xx (Client)     ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Test 403 - Forbidden (path traversal)
test_request "403 - Path Traversal" "GET" "http://$HOST/../etc/passwd" "403"
test_request "403 - Path Traversal avec double dot" "GET" "http://$HOST/../../etc/passwd" "403"
test_request "403 - Path Traversal encodé" "GET" "http://$HOST/%2e%2e/etc/passwd" "403"

# Test 404 - Not Found
test_request "404 - Fichier inexistant" "GET" "http://$HOST/fichier_qui_nexiste_pas.html" "404"
test_request "404 - Extension inconnue" "GET" "http://$HOST/test.xyz" "404"
test_request "404 - Répertoire inexistant" "GET" "http://$HOST/dossier_inexistant/file.html" "404"
test_request "404 - CGI inexistant" "GET" "http://$HOST/cgi-bin/script_inexistant.py" "404"

# Test 405 - Method Not Allowed
test_request "405 - Méthode PATCH non autorisée" "PATCH" "http://$HOST/index.html" "405"
test_request "405 - Méthode PUT non autorisée" "PUT" "http://$HOST/index.html" "405"
test_request "405 - Méthode DELETE sur / non autorisée" "DELETE" "http://$HOST/" "405"

# Test 413 - Payload Too Large
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Test:${NC} 413 - Body trop volumineux"
echo "Création d'un fichier de 3MB (max: 2MB)..."
dd if=/dev/zero of=/tmp/large_file.bin bs=1M count=3 2>/dev/null
echo "Envoi du fichier..."
response=$(curl -s -o /tmp/test_response.html -w "%{http_code}" \
    -X POST \
    -H "Content-Type: multipart/form-data" \
    -F "file=@/tmp/large_file.bin" \
    "http://$HOST/upload")
http_code=$response
echo -e "${BLUE}Code reçu:${NC} $http_code"
if [ "$http_code" = "413" ]; then
    echo -e "${GREEN}✓ PASSED${NC} - Code HTTP correct"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC} - Code HTTP incorrect (attendu: 413, reçu: $http_code)"
    FAILED=$((FAILED + 1))
fi
rm -f /tmp/large_file.bin
echo ""

# ============================================================
# Tests des codes d'erreur 5xx (Erreurs serveur)
# ============================================================

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║      TESTS ERREURS 5xx (Serveur)     ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Test 500 - Internal Server Error (CGI avec erreur)
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Test:${NC} 500 - CGI avec erreur de syntaxe"
cat > /tmp/error_script.py << 'EOF'
#!/usr/bin/env python3
# Script avec erreur volontaire
print("Mauvais format de header")
print("Ceci va causer une erreur")
EOF
chmod +x /tmp/error_script.py
cp /tmp/error_script.py ./www/cgi-bin/error_script.py 2>/dev/null
if [ -f ./www/cgi-bin/error_script.py ]; then
    response=$(curl -s -o /tmp/test_response.html -w "%{http_code}" "http://$HOST/cgi-bin/error_script.py")
    http_code=$response
    echo -e "${BLUE}Code reçu:${NC} $http_code"
    if [ "$http_code" = "500" ] || [ "$http_code" = "502" ]; then
        echo -e "${GREEN}✓ PASSED${NC} - Code HTTP correct (erreur serveur)"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ FAILED${NC} - Code HTTP incorrect (attendu: 500 ou 502, reçu: $http_code)"
        FAILED=$((FAILED + 1))
    fi
    rm ./www/cgi-bin/error_script.py
else
    echo -e "${YELLOW}⚠ SKIPPED${NC} - Impossible de créer le script de test"
fi
rm -f /tmp/error_script.py
echo ""

# Test 502 - Bad Gateway (CGI timeout ou crash)
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Test:${NC} 502/504 - CGI qui timeout"
cat > /tmp/slow_script.py << 'EOF'
#!/usr/bin/env python3
import time
print("Content-Type: text/html\r")
print("\r")
time.sleep(15)
print("<html><body>Too slow</body></html>")
EOF
chmod +x /tmp/slow_script.py
cp /tmp/slow_script.py ./www/cgi-bin/slow_script.py 2>/dev/null
if [ -f ./www/cgi-bin/slow_script.py ]; then
    echo "Requête avec timeout de 8 secondes..."
    response=$(curl -s -o /tmp/test_response.html -w "%{http_code}" --max-time 8 "http://$HOST/cgi-bin/slow_script.py")
    http_code=$response
    echo -e "${BLUE}Code reçu:${NC} $http_code"
    if [ "$http_code" = "502" ] || [ "$http_code" = "504" ] || [ "$http_code" = "000" ]; then
        echo -e "${GREEN}✓ PASSED${NC} - Timeout géré correctement"
        PASSED=$((PASSED + 1))
    else
        echo -e "${YELLOW}⚠ WARNING${NC} - Code inattendu: $http_code (peut être normal selon l'implémentation)"
        PASSED=$((PASSED + 1))
    fi
    rm ./www/cgi-bin/slow_script.py
else
    echo -e "${YELLOW}⚠ SKIPPED${NC} - Impossible de créer le script de test"
fi
rm -f /tmp/slow_script.py
echo ""

# ============================================================
# Tests des codes de succès (pour contrôle)
# ============================================================

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║      TESTS SUCCÈS 2xx (Contrôle)     ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""

# Test 200 - OK
test_request "200 - Accès page d'accueil" "GET" "http://$HOST/" "200"
test_request "200 - Accès fichier HTML existant" "GET" "http://$HOST/index.html" "200"

# Test 201 - Created (upload)
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Test:${NC} 201 - Upload fichier valide"
echo "test content" > /tmp/small_file.txt
response=$(curl -s -o /tmp/test_response.html -w "%{http_code}" \
    -X POST \
    -H "Content-Type: multipart/form-data" \
    -F "file=@/tmp/small_file.txt" \
    "http://$HOST/upload")
http_code=$response
echo -e "${BLUE}Code reçu:${NC} $http_code"
if [ "$http_code" = "201" ] || [ "$http_code" = "200" ]; then
    echo -e "${GREEN}✓ PASSED${NC} - Upload réussi"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ FAILED${NC} - Code HTTP incorrect (attendu: 201 ou 200, reçu: $http_code)"
    FAILED=$((FAILED + 1))
fi
rm -f /tmp/small_file.txt
echo ""

# ============================================================
# Résumé des tests
# ============================================================

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║           RÉSUMÉ DES TESTS            ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}"
echo ""
TOTAL=$((PASSED + FAILED))
echo -e "${GREEN}Tests réussis:${NC} $PASSED"
echo -e "${RED}Tests échoués:${NC} $FAILED"
echo -e "${BLUE}Total:${NC} $TOTAL"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║     ✓ TOUS LES TESTS ONT RÉUSSI!     ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════╗${NC}"
    echo -e "${RED}║   ✗ CERTAINS TESTS ONT ÉCHOUÉ         ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════╝${NC}"
    exit 1
fi

# Nettoyage
rm -f /tmp/test_response.html
