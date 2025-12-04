#!/bin/bash
# filepath: /home/acrusoe/Documents/Webserv/test_errors.sh

echo "=== Test des pages d'erreur du serveur ==="
echo ""

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PORT=8080
HOST="localhost:$PORT"

# Fonction pour tester une URL
test_url() {
    local url=$1
    local expected_code=$2
    local description=$3
    
    echo -e "${YELLOW}Test: $description${NC}"
    echo "URL: $url"
    
    # Récupérer le code HTTP et le Content-Type
    response=$(curl -s -o /tmp/response.html -w "%{http_code}|%{content_type}" "$url")
    http_code=$(echo $response | cut -d'|' -f1)
    content_type=$(echo $response | cut -d'|' -f2)
    
    echo "Code HTTP reçu: $http_code"
    echo "Content-Type: $content_type"
    
    # Vérifier le code HTTP
    if [ "$http_code" = "$expected_code" ]; then
        echo -e "${GREEN}✓ Code HTTP correct${NC}"
    else
        echo -e "${RED}✗ Code HTTP incorrect (attendu: $expected_code)${NC}"
    fi
    
    # Vérifier le Content-Type
    if [[ "$content_type" == *"text/html"* ]]; then
        echo -e "${GREEN}✓ Content-Type correct (HTML)${NC}"
    else
        echo -e "${RED}✗ Content-Type incorrect: $content_type${NC}"
        echo -e "${RED}  → Le navigateur va télécharger au lieu d'afficher!${NC}"
    fi
    
    # Afficher un extrait de la réponse
    echo "Extrait de la réponse:"
    head -n 5 /tmp/response.html
    echo ""
    echo "---"
    echo ""
}

# Tests des erreurs
echo "Démarrez votre serveur avec: ./a.out ./conf/webserv.conf"
echo "Appuyez sur Entrée pour commencer les tests..."
read

echo ""
echo "=== 1. Test 404 - Fichier inexistant ==="
test_url "http://$HOST/fichier_inexistant.html" "404" "Fichier qui n'existe pas"

echo ""
echo "=== 2. Test 404 - Répertoire CGI sans fichier ==="
test_url "http://$HOST/cgi-bin" "404" "Répertoire cgi-bin seul"
test_url "http://$HOST/cgi-bin/" "404" "Répertoire cgi-bin avec slash"

echo ""
echo "=== 3. Test 404 - CGI inexistant ==="
test_url "http://$HOST/cgi-bin/inexistant.py" "404" "Script CGI inexistant"

echo ""
echo "=== 4. Test 403 - Path traversal ==="
test_url "http://$HOST/../etc/passwd" "403" "Tentative d'accès à /etc/passwd"

echo ""
echo "=== 5. Test 405 - Méthode non autorisée ==="
echo -e "${YELLOW}Test: Méthode PATCH non supportée${NC}"
curl -s -o /tmp/response.html -w "Code HTTP: %{http_code}\nContent-Type: %{content_type}\n" -X PATCH "http://$HOST/index.html"
echo ""

echo ""
echo "=== 6. Test 413 - Body trop grand ==="
echo -e "${YELLOW}Test: Upload d'un fichier trop volumineux${NC}"
# Créer un fichier de 2MB (ajustez selon votre max_body_size)
dd if=/dev/zero of=/tmp/big_file.bin bs=1M count=2 2>/dev/null
curl -s -o /tmp/response.html -w "Code HTTP: %{http_code}\nContent-Type: %{content_type}\n" \
    -F "file=@/tmp/big_file.bin" "http://$HOST/upload"
rm /tmp/big_file.bin
echo ""

echo ""
echo "=== 7. Test 504 - CGI Timeout ==="
echo -e "${YELLOW}Test: Script CGI qui prend trop de temps${NC}"
# Créez un script qui sleep pendant 10 secondes
cat > /tmp/slow.py << 'EOF'
#!/usr/bin/env python3
import time
print("Content-Type: text/html\r")
print("\r")
time.sleep(10)
print("<html><body>Should timeout</body></html>")
EOF
chmod +x /tmp/slow.py
cp /tmp/slow.py ./www/cgi-bin/slow.py
curl -s -o /tmp/response.html -w "Code HTTP: %{http_code}\nContent-Type: %{content_type}\n" \
    "http://$HOST/cgi-bin/slow.py" --max-time 10
rm ./www/cgi-bin/slow.py /tmp/slow.py
echo ""

echo ""
echo "=== 8. Test 500 - CGI avec erreur ==="
echo -e "${YELLOW}Test: Script CGI qui retourne une erreur${NC}"
cat > /tmp/error.py << 'EOF'
#!/usr/bin/env python3
print("Invalid header format")
print("This should cause error")
EOF
chmod +x /tmp/error.py
cp /tmp/error.py ./www/cgi-bin/error.py
curl -s -o /tmp/response.html -w "Code HTTP: %{http_code}\nContent-Type: %{content_type}\n" \
    "http://$HOST/cgi-bin/error.py"
rm ./www/cgi-bin/error.py /tmp/error.py
echo ""

echo ""
echo "=== Résumé ==="
echo "Vérifiez que:"
echo "1. Tous les codes HTTP sont corrects"
echo "2. Tous les Content-Type sont 'text/html'"
echo "3. Les pages s'affichent dans le navigateur (pas de téléchargement)"
echo ""
echo "Pour tester manuellement dans le navigateur:"
echo "  - http://$HOST/inexistant.html (404)"
echo "  - http://$HOST/cgi-bin (404)"
echo "  - http://$HOST/../etc/passwd (403)"

rm -f /tmp/response.html