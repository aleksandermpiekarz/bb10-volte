# BlackBerry OS 10 — AutoRoot v4 i SSH na Windows 11

**Zakres:** od przygotowania narzędzi do zalogowania przez SSH jako `root`. Bez kompilacji i instalacji VoLTE.

**Sprawdzone w praktyce:** BlackBerry Q10 **SQN100-3**, oprogramowanie **10.3.3.1463** (obraz OS 10.3.3.2205), Windows 11, AutoRoot v4, `bb10mt`, USB. Użycie na innym modelu lub wersji OS wymaga sprawdzenia zgodności firmware i paczki AutoRoot. Nie przenoś autoloadera Q10 na inne modele.

> **Ostrzeżenie:** flashowanie usuwa dane i może spowodować problemy z uruchomieniem, modemem lub całkowicie unieruchomić telefon. Standardowy backup danych BB10 nie jest kopią wszystkich partycji, zwłaszcza NV/EFS. Zachowaj oryginalny, zgodny autoloader jako opcję odzyskiwania; nie daje on gwarancji naprawy. Nie uruchamiaj cudzych `.exe`, DLL ani skryptów bez sprawdzenia pochodzenia. Domyślne SSH AutoRoot korzysta ze współdzielonego klucza — po teście zastąp go własnym.

## 1. Wymagania i pliki

- Komputer Windows 11, sprawny **kabel micro-USB z transmisją danych**, bateria telefonu najlepiej ≥70%.
- **BlackBerry 10 Desktop Software Setup** — zainstaluj przede wszystkim **Device Drivers / BlackBerry Device Manager**. Możesz odznaczyć `BlackBerry Blend` i `BlackBerry Link`, **nie odinstalowuj sterowników**. Po instalacji najlepiej uruchom Windows ponownie. Zamknij otwarty BlackBerry Link i Sachesi podczas flashowania, ale nie usuwaj sterowników ani działającej usługi Device Manager.
- **AutoRoot v4**: https://bb10.root.sx/blog/autoroot-v4 — pobierz archiwum autora.
- **Oryginalny autoloader pasujący do modelu i wersji systemu**. Dla *testowanego Q10 SQN100-3* użyto pliku `Q10_10.3.03.2205_SQN100-1-2-3-4-5.exe`; sama nazwa pliku nie dowodzi jego autentyczności ani zgodności z innymi urządzeniami. Zachowaj kopię oryginału.
- `bb10mt.exe` oraz pełny katalog `patchdata` z AutoRoot v4.
- Biblioteki Windows, jeśli narzędzie zgłosi ich brak: `libusb-1.0.dll` (libusb: https://github.com/libusb/libusb/releases) i `lzo2.dll` (LZO2, np. https://packages.msys2.org/packages/mingw-w64-x86_64-lzo2). Użyj wersji **64-bitowych**, kompatybilnych z `bb10mt.exe`, pochodzących z zaufanych dystrybucji; nie pobieraj z losowych katalogów DLL. W MSYS2 biblioteka może występować jako `liblzo2-2.dll` — sprawdź dokumentację/nazwę wymaganą przez konkretny build `bb10mt` przed wykonaniem kopii pod nazwą `lzo2.dll`.

Przykładowa organizacja folderów:

```text
D:\blackberry\
├── autoroot_v4\
│   ├── bb10mt.exe
│   ├── patch.txt
│   ├── patchdata\
│   ├── libusb-1.0.dll     (jeśli wymagana)
│   └── lzo2.dll          (jeśli wymagana)
├── Q10_10.3.03.2205_SQN100-1-2-3-4-5.exe
├── q10root\             (wynik batch)
└── keys\
    └── id_rsa4x         (klucz *klienta* SSH z paczki AutoRoot)
```

## 2. Sprawdź `patch.txt`

Nie zastępuj oryginalnego `patch.txt` kilkoma linijkami — zawiera także `push`, `chmod`, `chown`, konfigurację usług i pliki potrzebne rootowi. W testowanej paczce obecne były:

```text
mkdir -p "/var/rootdata"
touch /var/rootdata/use.root
touch /var/rootdata/use.sshd
#touch /var/rootdata/use.berrycore
push "patchdata/apps" "/apps"
push "patchdata/var" "/var"
```

`use.root` włącza mechanizm roota, `use.sshd` autostart serwera SSH. BerryCore pozostawiony wyłączony. Pozostałe instrukcje z oryginalnej paczki pozostaw bez zmian, o ile nie wiesz dokładnie, co robią. Włączenie SSH z kluczem publikowanym wraz z projektem **nie jest bezpieczną konfiguracją na stałe**; używaj połączenia USB, unikaj wystawiania portu 22 do Wi-Fi/internetu.

## 3. Przygotuj zmodyfikowane obrazy (bez flashowania)

W PowerShell:

```powershell
cd D:\blackberry\autoroot_v4
Test-Path .\bb10mt.exe
Test-Path .\patch.txt
Test-Path .\patchdata\apps
Test-Path .\patchdata\var
```

Każde powinno zwrócić `True`. Następnie **tylko jeśli obraz pasuje do Twojego modelu**:

```powershell
.\bb10mt.exe batch `
  -i "D:\blackberry\Q10_10.3.03.2205_SQN100-1-2-3-4-5.exe" `
  -s ".\patch.txt" `
  -o "D:\blackberry\q10root"
```

**Uwaga:** dla innego telefonu zmień wejściowy obraz, ścieżkę wynikową i najpierw sprawdź zgodność jego wariantu z AutoRoot. W testowanej próbie log zawierał `Extraction completed successfully`, `Successfully touched file .../use.root`, `.../use.sshd`, `Packing files into OS autoloader`, `Packing files into Radio autoloader`.

### Częste błędy przy `bb10mt`

- `Couldn't load dynamic library libusb` → brakuje kompatybilnej biblioteki `libusb` w katalogu programu albo na ścieżce wyszukiwania DLL.
- `Couldn't load dynamic library liblzo2` → brakuje kompatybilnej `lzo2.dll`. W naszym przypadku po naprawie `libusb` pojawił się właśnie drugi komunikat.
- `WARNING! THIS PROGRAM LEAKS MEMORY!` po wyjątku → diagnostyka awaryjnego wyjścia, **nie** dowód uszkodzenia telefonu.

## 4. Sprawdź pliki wynikowe

```powershell
Get-ChildItem "D:\blackberry\q10root" -Recurse -File |
  Select-Object Name, Length, FullName

Get-ChildItem "D:\blackberry\q10root" -Recurse -File |
  Where-Object { $_.Length -eq 0 }

Get-ChildItem "D:\blackberry\q10root" -Recurse -File |
  Get-FileHash -Algorithm SHA256
```

W udanej próbie powstały m.in.:

```text
Q10_10.3.03.2205_SQN100-1-2-3-4-5_os.exe       ~3.02 GB
Q10_10.3.03.2205_SQN100-1-2-3-4-5_radio.exe    ~39.8 MB
```

Oprócz nich tworzone są pośrednie obrazy `.ifs`, `.rcfs`, `.sig2`, `.mbr`, `.ufs`, `.signed`, `.unsigned`, `.lst`. SHA-256 umożliwia wykrycie późniejszych zmian plików, ale nie dowodzi, że są bezpieczne, oficjalne ani uruchamialne.

## 5. Flashowanie OS i radia — procedura wysokiego ryzyka

**Wykonuj wyłącznie na urządzeniu, dla którego obrazy zostały przygotowane.** Zamknij Sachesi i okno BlackBerry Link; **BlackBerry Device Manager oraz sterowniki USB muszą zostać zainstalowane**. Nie odinstalowuj ich. Podłącz telefon bezpośrednio do portu USB. Zadbaj o zasilanie i nie odłączaj kabla podczas zapisu.

1. Uruchom wygenerowany `*_os.exe` z uprawnieniami administratora, najlepiej z terminala, by zachować ewentualny błąd i kod wyjścia. Samo zamknięcie konsoli **nie potwierdza sukcesu**.
2. Dokumentacja AutoRoot v4 opisuje etap `*_radio.exe` **po OS** i dopuszcza pewien końcowy błąd autoloadera OS. Nie zakładaj jednak, że *każdy* błąd jest normalny: zweryfikuj etap, komunikat i zgodność z instrukcją autora przed przejściem dalej.
3. Uruchom pasujący `*_radio.exe` zgodnie z procedurą AutoRoot. To również zapis firmware urządzenia; błędny obraz radia może uszkodzić łączność komórkową.
4. Sprawdź, czy telefon normalnie się uruchamia. Podczas naszej próby po `_os.exe` telefon migał czerwoną diodą w sekwencji **1–3–2** i nie startował, ale po uruchomieniu odpowiadającego `_radio.exe` uruchomił się prawidłowo. **To opis konkretnego przypadku, nie reguła ani gwarancja naprawy.**

W razie `Cannot connect to BlackBerry Device Manager` przerwij próbę i przywróć pakiet **Device Drivers** z instalatora BlackBerry 10 Desktop Software, uruchom ponownie Windows, sprawdź połączenie. W naszym przypadku wcześniejsze odinstalowanie BlackBerry Link usunęło potrzebny komponent — nie należy tego powtarzać.

## 6. Sprawdź komunikację USB i adres Q10

Po uruchomieniu telefonu podłącz go do Windows. W PowerShell:

```powershell
Get-NetAdapter | Format-Table Name, Status, InterfaceDescription
Get-NetIPAddress -AddressFamily IPv4 |
  Format-Table InterfaceAlias, IPAddress
```

W **naszej** próbie:

```text
Ethernet 2   Up   BlackBerry Device   → Windows: 169.254.0.2
Telefon BlackBerry Q10                → 169.254.0.1
```

Telefon może mieć inny adres. `169.254.0.1` był poprawną hipotezą dla tej konkretnej pary interfejsów, **nie uniwersalnym adresem każdego BB10**.

Test portu SSH:

```powershell
Test-NetConnection 169.254.0.1 -Port 22
```

Oczekiwane: `TcpTestSucceeded : True`. Jeśli `False`, sprawdź kabel, odblokowanie telefonu, interfejs Windows, trasę IP i czy `use.sshd` jest rzeczywiście obecny po instalacji. Włączenie `Development Mode` w Ustawienia → Zabezpieczenia i prywatność → Tryb programisty **nie jest wymagane** do SSH uruchomionego przez AutoRoot.

## 7. Znajdź właściwy klucz SSH

AutoRoot v4 używa klucza klienta **`id_rsa4x`** opisanego w dokumentacji. W naszej rozpakowanej `autoroot_v4` ten plik nie występował — trzeba było odnaleźć go osobno w plikach dystrybucji. Umieść go w:

```text
D:\blackberry\keys\id_rsa4x
```

**Nie myl go z:**

```text
patchdata\var\etc\ssh\ssh_host_rsa_key
patchdata\var\etc\ssh\ssh_host_rsa_key.pub
```

To klucze **serwera**, a nie prywatny klucz użytkownika do logowania. Na urządzeniu `sshd_config` zawierało:

```text
AllowUsers=root
PermitRootLogin=yes
PasswordAuthentication=no
AuthorizedKeysFile=/etc/ssh/authorized_keys2 /accounts/devuser/.ssh/authorized_keys2
```

Nie przesyłaj nikomu prywatnego klucza. Klucz `id_rsa4x` z publikowanej paczki należy traktować jako **niezaufany do długotrwałego użytku**.

## 8. Uprawnienia prywatnego klucza w Windows

Nowy OpenSSH odrzuca plik, jeśli grupa `Authenticated Users` albo `BUILTIN\Users` ma do niego dostęp. W PowerShell uruchomionym na koncie właściciela klucza:

```powershell
$key = "D:\blackberry\keys\id_rsa4x"
$user = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name

icacls $key /inheritance:r
icacls $key /remove:g "*S-1-5-11"
icacls $key /remove:g "*S-1-5-32-545"
icacls $key /grant:r "${user}:R"
icacls $key
```

Opcje usuwają dziedziczone/szerokie uprawnienia; w szczególnych konfiguracjach dysku może być potrzebne dodatkowe dostosowanie ACL. Upewnij się, że *Twój* użytkownik ma odczyt. Nie stosuj komend ACL do całego katalogu ani dysku.

## 9. Połączenie przez SSH

Stary `sshd` w Q10 oferował host key `ssh-rsa` i algorytmy MAC m.in. `hmac-sha1`. Współczesny OpenSSH wymagał jawnego dopuszczenia tych algorytmów **dla jednej sesji**:

```powershell
ssh -i "D:\blackberry\keys\id_rsa4x" `
  -o HostKeyAlgorithms=+ssh-rsa `
  -o PubkeyAcceptedAlgorithms=+ssh-rsa `
  -o MACs=+hmac-sha1 `
  root@169.254.0.1
```

Przy pierwszym połączeniu sprawdź fingerprint serwera względem `patchdata\var\etc\ssh\ssh_host_rsa_key.pub`, dopiero potem zaakceptuj (`yes`). W naszej sesji serwer zgłosił fingerprint `SHA256:m1a0gz/Wm19JTrXwheFQ1MvMnrlBipROh0stttu9npw`, ale **nie używaj go jako bezwarunkowego wzorca dla innej instalacji** (klucze serwera mogą być inne).

W naszej próbie:

- Bez dodatkowych opcji: `no matching host key type ... ssh-rsa`.
- Po dodaniu `HostKeyAlgorithms` i `PubkeyAcceptedAlgorithms`: `no matching MAC found`.
- Po dodaniu `MACs=+hmac-sha1`: komunikat `UNPROTECTED PRIVATE KEY FILE`, naprawiony poleceniami `icacls` powyżej.
- Po naprawie ACL połączenie SSH zakończyło się sukcesem.

Te starsze algorytmy nie powinny być udostępniane do połączeń przez publiczną sieć. Na koniec ogranicz dostęp do SSH i zastąp współdzielony klucz prywatnym kluczem wygenerowanym przez siebie, aktualizując `authorized_keys2`.

## 10. Potwierdzenie działania po SSH

W konsoli BlackBerry pojawia się znak zachęty `#`. W naszej próbie:

```sh
echo $USER
uname -a
echo $PATH
ls -l /var/rootdata
ls -l /var/addons
```

Przykładowe wyniki z działającego Q10:

```text
# echo $USER
root
# uname -a
QNX BLACKBERRY-7B11 8.0.0 2016/12/12-22:40:53EST MSM8960_V3.2.1.1_F_R085_Rev:17 armle
```

`uname` pokazuje wewnętrzną wersję QNX, nie numer wersji BlackBerry OS. W `/var/rootdata` były m.in. `installer`, `install.sh`, `Term49.bar`, `use.root`, `use.sshd`. Brak `/var/addons` przed instalacją VoLTE jest oczekiwany.

Polecenie `id` zwróciło u nas `sh: id: cannot execute - No such file or directory`, a zmienna `$UID` była pusta; sama ich niedostępność nie oznacza braku roota. Logowanie przez SSH użytkownika root oraz obecność tych plików potwierdziły działanie naszej konfiguracji, choć `echo $USER` samo w sobie nie jest niezależnym pomiarem UID procesu.

## Stop — tu kończy się ta instrukcja

Na tym etapie: telefon uruchamia się po AutoRoot v4, SSH na USB działa, a konsola QNX jest dostępna jako `root`. **Nie obejmuje** instalacji ani kompilacji `bb10-volte` (PR #5), zmiany IMS/APN, VoLTE czy modyfikacji modemu.

### Źródła techniczne i pewność

- [AutoRoot v4 — dokumentacja autora](https://bb10.root.sx/blog/autoroot-v4) — **8/10** dla zamierzonego działania i procedury; bezpieczeństwo i zgodność z innymi modelami nie są gwarantowane.
- [bb10mt — kod/narzędzie](https://github.com/bb10root/bb10mt) — **8/10** dla składni narzędzia; niezależna kontrola wygenerowanych obrazów nie została przeprowadzona.
- [libusb — oficjalne wydania](https://github.com/libusb/libusb/releases) — **9/10** dla bibliotek libusb.
- [MSYS2 LZO2](https://packages.msys2.org/packages/mingw-w64-x86_64-lzo2) — **9/10** dla dystrybucji pakietu LZO2.

Dokument opiera się przede wszystkim na **zaobserwowanej sesji instalacji**. Nie są to publikacje recenzowane naukowo, a sukces na Q10 SQN100-3 nie przesądza o działaniu na drugim egzemplarzu lub innym modelu.
