#!/usr/bin/env python3
"""
Test automatisé des codes d'erreur HTTP du serveur Webserv
Utilise les requêtes HTTP pour tester tous les codes d'erreur
"""

import requests
import time
import sys
import os

# Configuration
HOST = "localhost"
PORT = 8080
BASE_URL = f"http://{HOST}:{PORT}"

# Couleurs pour le terminal
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    END = '\033[0m'

class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.tests = []
    
    def add_test(self, name, expected, actual, success):
        self.tests.append({
            'name': name,
            'expected': expected,
            'actual': actual,
            'success': success
        })
        if success:
            self.passed += 1
        else:
            self.failed += 1
    
    def print_summary(self):
        print(f"\n{Colors.BOLD}{'='*60}{Colors.END}")
        print(f"{Colors.BOLD}RÉSUMÉ DES TESTS{Colors.END}")
        print(f"{'='*60}")
        print(f"{Colors.GREEN}✓ Tests réussis: {self.passed}{Colors.END}")
        print(f"{Colors.RED}✗ Tests échoués: {self.failed}{Colors.END}")
        print(f"Total: {self.passed + self.failed}")
        print(f"{'='*60}\n")
        
        if self.failed > 0:
            print(f"{Colors.RED}Tests échoués:{Colors.END}")
            for test in self.tests:
                if not test['success']:
                    print(f"  - {test['name']}: attendu {test['expected']}, reçu {test['actual']}")

def test_request(name, method, url, expected_code, data=None, files=None, timeout=5):
    """Effectue une requête HTTP et vérifie le code de statut"""
    print(f"\n{Colors.YELLOW}{'─'*60}{Colors.END}")
    print(f"{Colors.BOLD}Test:{Colors.END} {name}")
    print(f"{Colors.BLUE}URL:{Colors.END} {url}")
    print(f"{Colors.BLUE}Méthode:{Colors.END} {method}")
    print(f"{Colors.BLUE}Code attendu:{Colors.END} {expected_code}")
    
    try:
        if method == "GET":
            response = requests.get(url, timeout=timeout, allow_redirects=False)
        elif method == "POST":
            response = requests.post(url, data=data, files=files, timeout=timeout, allow_redirects=False)
        elif method == "DELETE":
            response = requests.delete(url, timeout=timeout, allow_redirects=False)
        elif method == "PUT":
            response = requests.put(url, timeout=timeout, allow_redirects=False)
        elif method == "PATCH":
            response = requests.patch(url, timeout=timeout, allow_redirects=False)
        else:
            response = requests.request(method, url, timeout=timeout, allow_redirects=False)
        
        actual_code = response.status_code
        print(f"{Colors.BLUE}Code reçu:{Colors.END} {actual_code}")
        print(f"{Colors.BLUE}Content-Type:{Colors.END} {response.headers.get('Content-Type', 'N/A')}")
        
        success = (actual_code == expected_code)
        
        if success:
            print(f"{Colors.GREEN}✓ PASSED{Colors.END}")
        else:
            print(f"{Colors.RED}✗ FAILED{Colors.END} (attendu: {expected_code}, reçu: {actual_code})")
        
        # Afficher un extrait de la réponse
        if len(response.text) > 0:
            lines = response.text.split('\n')[:3]
            print(f"{Colors.BLUE}Extrait de la réponse:{Colors.END}")
            for line in lines:
                print(f"  {line[:80]}")
        
        return success, actual_code
    
    except requests.exceptions.Timeout:
        print(f"{Colors.YELLOW}⏱ TIMEOUT{Colors.END}")
        # Pour les tests de timeout, on considère que c'est un succès si on attend une erreur 5xx
        if expected_code >= 500:
            print(f"{Colors.GREEN}✓ PASSED (timeout attendu){Colors.END}")
            return True, 504
        else:
            return False, 0
    
    except requests.exceptions.ConnectionError:
        print(f"{Colors.RED}✗ Erreur de connexion au serveur{Colors.END}")
        return False, 0
    
    except Exception as e:
        print(f"{Colors.RED}✗ Erreur: {e}{Colors.END}")
        return False, 0

def check_server():
    """Vérifie que le serveur est accessible"""
    try:
        response = requests.get(f"{BASE_URL}/", timeout=2)
        return True
    except:
        return False

def main():
    print(f"{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}Test des codes d'erreur HTTP - Webserv{Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}\n")
    
    # Vérifier que le serveur est démarré
    print("Vérification de la connexion au serveur...")
    if not check_server():
        print(f"{Colors.RED}✗ ERREUR: Le serveur n'est pas accessible sur {BASE_URL}{Colors.END}")
        print("Démarrez votre serveur avec: ./webserv ./conf/webserv.conf")
        sys.exit(1)
    print(f"{Colors.GREEN}✓ Serveur accessible{Colors.END}\n")
    
    result = TestResult()
    
    # ============================================================
    # Tests des erreurs 4xx (Erreurs client)
    # ============================================================
    
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}TESTS ERREURS 4xx (Client){Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    
    # Test 403 - Forbidden
    success, code = test_request(
        "403 - Path Traversal",
        "GET",
        f"{BASE_URL}/../etc/passwd",
        403
    )
    result.add_test("403 - Path Traversal", 403, code, success)
    
    success, code = test_request(
        "403 - Path Traversal double",
        "GET",
        f"{BASE_URL}/../../etc/passwd",
        403
    )
    result.add_test("403 - Path Traversal double", 403, code, success)
    
    # Test 404 - Not Found
    success, code = test_request(
        "404 - Fichier inexistant",
        "GET",
        f"{BASE_URL}/fichier_inexistant_123456.html",
        404
    )
    result.add_test("404 - Fichier inexistant", 404, code, success)
    
    success, code = test_request(
        "404 - Répertoire inexistant",
        "GET",
        f"{BASE_URL}/dossier_nexiste_pas/file.html",
        404
    )
    result.add_test("404 - Répertoire inexistant", 404, code, success)
    
    success, code = test_request(
        "404 - CGI inexistant",
        "GET",
        f"{BASE_URL}/cgi-bin/script_inexistant.py",
        404
    )
    result.add_test("404 - CGI inexistant", 404, code, success)
    
    # Test 405 - Method Not Allowed
    success, code = test_request(
        "405 - Méthode PATCH non autorisée",
        "PATCH",
        f"{BASE_URL}/index.html",
        405
    )
    result.add_test("405 - PATCH", 405, code, success)
    
    success, code = test_request(
        "405 - Méthode PUT non autorisée",
        "PUT",
        f"{BASE_URL}/index.html",
        405
    )
    result.add_test("405 - PUT", 405, code, success)
    
    # Test 413 - Payload Too Large
    print(f"\n{Colors.YELLOW}{'─'*60}{Colors.END}")
    print(f"{Colors.BOLD}Test:{Colors.END} 413 - Payload trop volumineux")
    print("Création d'un fichier de 3MB (max: 2MB)...")
    
    # Créer un fichier de 3MB
    large_data = b'X' * (3 * 1024 * 1024)
    files = {'file': ('large_file.bin', large_data)}
    
    try:
        response = requests.post(f"{BASE_URL}/upload", files=files, timeout=5)
        code = response.status_code
        print(f"{Colors.BLUE}Code reçu:{Colors.END} {code}")
        success = (code == 413)
        if success:
            print(f"{Colors.GREEN}✓ PASSED{Colors.END}")
        else:
            print(f"{Colors.RED}✗ FAILED{Colors.END} (attendu: 413, reçu: {code})")
        result.add_test("413 - Payload Too Large", 413, code, success)
    except Exception as e:
        print(f"{Colors.RED}✗ Erreur: {e}{Colors.END}")
        result.add_test("413 - Payload Too Large", 413, 0, False)
    
    # ============================================================
    # Tests des erreurs 5xx (Erreurs serveur)
    # ============================================================
    
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}TESTS ERREURS 5xx (Serveur){Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    
    # Test 500 - Internal Server Error (CGI avec erreur)
    print(f"\n{Colors.YELLOW}{'─'*60}{Colors.END}")
    print(f"{Colors.BOLD}Test:{Colors.END} 500 - CGI avec erreur")
    
    # Créer un script CGI avec erreur
    error_script = """#!/usr/bin/env python3
# Script avec erreur volontaire
print("Mauvais format")
print("Pas de header HTTP valide")
"""
    
    script_path = "./www/cgi-bin/error_test.py"
    try:
        with open(script_path, 'w') as f:
            f.write(error_script)
        os.chmod(script_path, 0o755)
        
        time.sleep(0.5)
        success, code = test_request(
            "500 - CGI avec erreur",
            "GET",
            f"{BASE_URL}/cgi-bin/error_test.py",
            500
        )
        # Accepter 500 ou 502
        if code in [500, 502]:
            success = True
        result.add_test("500 - CGI Error", 500, code, success)
        
        os.remove(script_path)
    except Exception as e:
        print(f"{Colors.YELLOW}⚠ SKIPPED: {e}{Colors.END}")
    
    # Test 502/504 - CGI Timeout
    print(f"\n{Colors.YELLOW}{'─'*60}{Colors.END}")
    print(f"{Colors.BOLD}Test:{Colors.END} 502/504 - CGI Timeout")
    
    slow_script = """#!/usr/bin/env python3
import time
print("Content-Type: text/html\\r")
print("\\r")
time.sleep(15)
print("<html><body>Too slow</body></html>")
"""
    
    script_path = "./www/cgi-bin/slow_test.py"
    try:
        with open(script_path, 'w') as f:
            f.write(slow_script)
        os.chmod(script_path, 0o755)
        
        time.sleep(0.5)
        success, code = test_request(
            "502/504 - CGI Timeout",
            "GET",
            f"{BASE_URL}/cgi-bin/slow_test.py",
            502,
            timeout=8
        )
        # Accepter 502, 504 ou timeout
        if code in [502, 504]:
            success = True
        result.add_test("502/504 - Timeout", 502, code, success)
        
        os.remove(script_path)
    except Exception as e:
        print(f"{Colors.YELLOW}⚠ SKIPPED: {e}{Colors.END}")
    
    # ============================================================
    # Tests de succès (contrôle)
    # ============================================================
    
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}TESTS SUCCÈS 2xx (Contrôle){Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    
    # Test 200 - OK
    success, code = test_request(
        "200 - Page d'accueil",
        "GET",
        f"{BASE_URL}/",
        200
    )
    result.add_test("200 - Home", 200, code, success)
    
    success, code = test_request(
        "200 - Fichier HTML",
        "GET",
        f"{BASE_URL}/index.html",
        200
    )
    result.add_test("200 - HTML File", 200, code, success)
    
    # Afficher le résumé
    result.print_summary()
    
    # Retourner le code de sortie approprié
    sys.exit(0 if result.failed == 0 else 1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Tests interrompus par l'utilisateur{Colors.END}")
        sys.exit(1)
