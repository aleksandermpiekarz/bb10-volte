# Projekt: VoLTE Orange Polska na BlackBerry Q10 — instrukcja dla agenta AI

**Data przygotowania:** 7 listopada 2026 r.\
**Cel dokumentu:** przekazać agentowi uruchomionemu **lokalnie w Ubuntu WSL2** pełny kontekst, zakres pracy, środowisko i kryteria powodzenia.\
**Dokument powiązany:** `blackberry_bb10_autoroot_ssh_instrukcja.md` — sprawdzona procedura od przygotowania obrazu AutoRoot v4 do logowania przez SSH jako `root`; jej nie powtarzamy tutaj.

## 1. Cel główny

Doprowadzić do działających **połączeń głosowych VoLTE przez Orange Polska** na **BlackBerry Q10 SQN100-3** z BlackBerry OS 10 i AutoRoot v4. Chodzi o prawdziwe rozmowy przychodzące i wychodzące przez LTE, **bez przechodzenia na 2G/EDGE**. Sam aktywny APN `ims`, stan `IMS_CONNECTED`, sygnalizacja SIP `200 OK` lub wskaźnik Berry **nie są wystarczającym dowodem** działania głosu VoLTE.

Na początek użytkownik chce podejść do problemu **od świeżo wgranego systemu** i **karty SIM Orange Polska**, bazując na **czystej, aktualnie zweryfikowanej gałęzi `main`** projektu:

- Repozytorium: https://github.com/bb10root/bb10-volte
- Szczególnie ważny PR #2 (Orange Polska): https://github.com/bb10root/bb10-volte/pull/2
- Oryginalny profil w repo: `pkg/data/carrier-149-volte.xml`
- Ustawienia IMS: `pkg/data/ir92-149`, m.in. `UsePreconditions:b:true`, `SMS_Format::3GPP`

**Najpierw czysty `main` i oryginalny profil Orange**, bez kopiowania dawnych konfiguracji T-Mobile i bez domyślnych lokalnych poprawek kodu. Każdą zmianę względem `main` należy wcześniej uzasadnić diagnostyką, zapisać w Git i uzyskać zgodę użytkownika.

PR #2 zawiera raport autora o działających rozmowach VoLTE w Orange Polska na **BlackBerry Classic**, co jest wartościowym punktem odniesienia, ale **nie gwarantuje identycznego wyniku na Q10**.

## 2. Środowisko robocze — agent działa w WSL, nie w PowerShell

- Windows 11, Ubuntu w **WSL2**, Docker z działającym obrazem toolchaina BB10.
- Użytkownik chce uruchomić **Codex CLI** w Ubuntu WSL i zlecać agentowi edycję repozytorium, budowanie ZIP, zdalną diagnostykę i przesyłanie plików.
- Projekt znajduje się w katalogach `~/projects/bb10-volte-original` (lokalna kopia projektu głównego) i dawniej `~/projects/bb10-volte` (eksperymenty z PR #5). Agent powinien **sam sprawdzić `git remote -v`, `git branch`, `git status`, `git log`**, zanim wykona reset lub kompilację.
- Oryginalny build, który przechodził w tej konfiguracji:

```bash
cd ~/projects/bb10-volte-original
sudo docker run --rm \
  -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  make CC=bb10-cc STRIP=llvm-strip
```

- Wynikiem jest `volte.zip` z `MANIFEST`, `data/` i `init/`.
- Telefon podłączony przez USB do **Windowsowego interfejsu BlackBerry Device**, zwykle telefon `169.254.0.1`, a Windows `169.254.0.2`. Adresy i dostępność mogą się zmienić po flashowaniu; należy je sprawdzić.
- Windowsowy klient OpenSSH działał z prywatnym kluczem klienta pod ścieżką `D:\blackberry\keys\id_rsa4x`. **Nie wyświetlaj, nie kopiuj do repo ani nie wysyłaj agentowi zawartości tego klucza.** Po reinstalacji należy zweryfikować fingerprint serwera.
- Dotychczas wymagana zgodność ze starymi algorytmami SSH Q10: `HostKeyAlgorithms=+ssh-rsa`, `PubkeyAcceptedAlgorithms=+ssh-rsa`, `MACs=+hmac-sha1`. Wykorzystuj je tylko w połączeniu USB do tego urządzenia.

### Windows OpenSSH uruchamiany bezpośrednio z WSL

WSL może uruchamiać `powershell.exe` i `scp.exe` ze środowiska Windows; nie trzeba ręcznie otwierać PowerShell. Agent może przygotować skrypty `bb-ssh`, `bb-send`, `bb-fetch` z bezpieczną obsługą argumentów, kodów wyjścia i cytowania ścieżek. Przykładowy transfer pliku z WSL:

```bash
# Z Ubuntu WSL
WIN_ZIP=$(wslpath -w "$(realpath ./volte.zip)")
# Windowsowy scp.exe musi otrzymać WIN_ZIP jako osobny, prawidłowo cytowany argument
```

Przykładowe **znane, działające polecenie Windows PowerShell**, z którym można porównać wygenerowany wrapper:

```powershell
scp -O `
  -i "D:\blackberry\keys\id_rsa4x" `
  -o HostKeyAlgorithms=+ssh-rsa `
  -o PubkeyAcceptedAlgorithms=+ssh-rsa `
  -o MACs=+hmac-sha1 `
  "D:\blackberry\volte.zip" `
  root@169.254.0.1:/var/rootdata/volte.zip
```

**Ważne:** nie zakładaj, że wskazany plik Windows istnieje; po zbudowaniu skrypt ma poprawnie przekonwertować ścieżkę z WSL. Nie wprowadzaj `sshpass`, nie wyłączaj bez potrzeby weryfikacji host key, nie kopiuj prywatnego klucza do repozytorium.

## 3. Punkty startowe Orange Polska

- MCC **260**, MNC **03** (w systemie może pojawić się liczba `3`).
- BlackBerry Carrier ID **149** — zostało zaobserwowane na badanym Q10 w `true_home_carrier` po włożeniu SIM Orange.
- Profil projektu: `carrier-149-volte.xml`; osobne ustawienia `ir92-149`.
- `pkg/data/perms` zawiera uprawnienia dla obu plików.
- Po włożeniu Orange poprzednio zaobserwowano identyfikację SIM jako `mcc=260,mnc=3,id=149`. **Nie jest to równoznaczne z rejestracją w sieci.**

### Dlaczego zaczynamy od fabrycznie czystego systemu

Wcześniejsze eksperymenty obejmowały różne rewizje dodatku, profile operatorów i wartości PPS. Użytkownik świadomie chce wyeliminować mieszanie środowisk. **Nie przedstawiaj wcześniejszych eksperymentów jako dowodu, że dana implementacja nie działa.** Mogły występować zależności konfiguracji, uprawnień, kolejności uruchamiania, stanu SIM i sieci. Starych wyników używaj jako wskazówek diagnostycznych, nie jako ostatecznych ocen.

**Po przywróceniu systemu nie instaluj od razu VoLTE**. Najpierw włóż Orange i sprawdź bazową usługę operatora bez dodatku: wykrycie SIM, rejestrację do Orange, SMS, działanie danych na LTE, możliwość zwykłych połączeń. Jeśli tu jest problem, diagnozuj najpierw rejestrację operatora, pasma i ustawienia telefonu; nie przypisuj winy IMS.

## 4. Zalecana sekwencja pracy dla agenta

1. Zapoznaj się z instrukcją AutoRoot/SSH z drugiego pliku Markdown. Użytkownik wykonuje/akceptuje flashowanie i wymianę SIM; agent nie flashuje bez wyraźnej zgody.
2. Po instalacji świeżego systemu potwierdź działanie telefonu **bez dodatku VoLTE** z kartą Orange. Zweryfikuj rejestrację, LTE, SMS i zwykłe rozmowy.
3. Zapewnij z Ubuntu WSL odczyt SSH i bezpieczny transport ZIP przez Windowsowego klienta SCP (skrypty). Zweryfikuj działanie `bb-ssh`, `bb-send`, `bb-fetch` na nieszkodliwym pliku testowym.
4. Zbuduj z czystego `main` projektu `bb10root/bb10-volte` z wbudowanym profilem Orange. Sprawdź `git status`, SHA commita, rozmiar binarki, listę plików ZIP i format `pkg/data/perms`.
5. Przed instalacją dodatku zapisz backup obecnej konfiguracji i odczytów PPS. Nie usuwaj systemowych profili operatorów, nie zmieniaj na stałe uprawnień do chronionych plików i nie dodawaj konfiguracji T-Mobile.
6. Zainstaluj ZIP metodą przewidzianą w AutoRoot v4, zweryfikuj, że aktywna binarka jest **dokładnie tą, która została skompilowana** (hash/rozmiar), a nie starszą wersją z tej samej nazwy pakietu.
7. Sprawdź automatyczny start `pkg/init/start.sh` po restarcie; sam fakt obecności skryptu **nie dowodzi**, że uruchamia się on we właściwym momencie. Zbierz logi i stan procesu, weryfikuj retry/recovery po wyjęciu/włożeniu SIM i przełączeniu sieci.
8. Diagnozuj kolejno: identyfikację operatora (`carrier_id=149`), profil APN `ims`, połączenie IP i P-CSCF, rejestrację IMS/SIP, następnie **rzeczywistą usługę głosową** i zestawianie rozmów na LTE.
9. Wykonaj próbę zwykłego połączenia na **drugi własny numer** tylko z użytkownikiem obecnym przy telefonie. Sprawdź wychodzące i przychodzące połączenie, brak CSFB do EDGE oraz dźwięk w obie strony. **Nie testuj numerów alarmowych.**
10. Jeżeli wymagane będą poprawki kodu, formułuj hipotezę, pokaż zmianę i test weryfikacyjny; każdą iterację commituj jako osobny etap, z opcją łatwego powrotu do czystego `main`.

## 5. Poprzednie obserwacje — kontekst, nie diagnoza końcowa

Wcześniej na Q10 z kartą T-Mobile Polska (MCC 260 / MNC 02, Carrier ID 240):

- Udało się uzyskać poprawnie podłączony APN `ims`, adres P-CSCF, stan IMS `CONNECTED`, a w części prób także SIP `200 OK`.
- Występowało przechodzenie rozmów na EDGE i okresowo rozłączenia IMS; to może mieć więcej niż jedną przyczynę.
- Odczyt `network_service:n:2097162` nie zawierał bitu `0x2000000`, który eksperymentalny PR #5 interpretuje jako wsparcie głosu IMS.
- Próby wymuszania bitu `force-ims-voice` zmieniały zachowanie telefonu, ale same nie stanowiły końcowego testu konfiguracji. **Nie włączaj wymuszania jako pierwszego kroku w Orange.**
- Oryginalny `main` bywał uruchamiany przy braku odczytu chronionego `/base/etc/carrier/ids/carrierIds.xml`, co prowadziło do `carrier_id=-1`; istnieją osobne dane `true_home_carrier` w PPS. Po czystym flashowaniu należy niezależnie zweryfikować, czy ten problem występuje i czy rzeczywiście wpływa na konfigurację Orange.
- `IMS_CONNECTED`, `INCOMING_READY` i `allowVoLteProvisioning:1` to stany pomocnicze; nie gwarantują działającej rozmowy VoLTE.
- W pewnym momencie po zmianie SIM Orange telefon wykrywał `true_home_carrier.id=149`, ale `data_services_status::OFF`, APN były `connectable:false`, a radio pokazywało `umts` i `serving_network` `260/01`. **Nie wolno na podstawie samego tego logu rozstrzygać**, czy problem powodowała konfiguracja dodatku, tryb sieci, rejestracja operatora, karta SIM czy firmware.

## 6. Cenne ścieżki diagnostyczne QNX/BB10

```sh
# Orange ID i profil systemowy
cat /pps/services/carriermanager/status_private

# Aktywne APN-y (ims/internet)
cat /pps/services/cds/cnice/status
cat /pps/services/cds/cnice/status_persistent

# Rejestracja radiowa i sygnał
cat /pps/services/cellular/radioctrl/status_cell_public

# IMS i usługi głosowe
cat /pps/services/ims/status_public
cat /pps/services/ims/cellular/status_private
cat /pps/services/ims/cellular/config
cat /pps/services/carrier_config/ims/ims

# Uruchomione procesy
pidin arg | grep -E 'ims_manager|ims_rrm'

# Proces zainstalowany przez AutoRoot
/var/addons/volte/init/ims_manager -a check -v
```

Uwaga: nie wszystkie pliki PPS są obecne w każdym stanie; część jest prywatna. `pidin`, `ls`, `cat` sprawdziły się w QNX. Narzędzia typowe dla GNU/Linux (`id`, `printf`, `systemctl`) mogą nie istnieć.

## 7. Bezpieczeństwo i zakres autonomii

- Agent może samodzielnie **czytać kod, dokumentację, logi**, proponować patche, budować w Dockerze i weryfikować artefakty.
- Agent ma wymagać **jawnej zgody przed** flashowaniem OS/radia, restartami telefonu/modemu, zatrzymaniem usług `cellular-*`, zmianą chronionych PPS/NVRAM, instalowaniem dodatku, zmianą uprawnień systemowych, uruchamianiem wymuszonego głosu i testami połączeń.
- Zawsze przygotuj kopie przed zmianami; używaj Git do checkpointów i sprawdzaj, czy backup da się odczytać.
- **Nie wykonuj testowych połączeń na 112 ani inne numery alarmowe.** W czasie eksperymentów użytkownik musi mieć dostępny drugi telefon ze sprawną łącznością.
- Logi mogą ujawniać numer telefonu (`impu`) i ICCID; przy zapisie lub udostępnieniu anonimizuj te wartości. Nie publikuj kluczy SSH ani poufnych identyfikatorów.
- Przed resetem `git clean -fdx` wyświetl listę potencjalnie usuwanych plików, jeśli w repo mogły powstać lokalne instrukcje, skrypty lub backupy.

## 8. Kryteria sukcesu i oczekiwany sposób raportowania

**Sukces techniczny:** Q10 na Orange Polska w LTE zestawia i odbiera zwykłe połączenia głosowe **bez przejścia na EDGE/2G**, a rozmowa ma działający dźwięk w obie strony. Dodatkowo konfiguracja przetrwa restart telefonu i nie pogarsza zwykłej łączności.

Po każdej iteracji agent podaje zwięźle:

- Co sprawdził lub zmienił (pliki, commit, skompilowany ZIP i jego hash).
- Stan sieci/IMS/głosu potwierdzony logami, z rozdzieleniem faktów od hipotez.
- Co jest następnym, możliwie najbezpieczniejszym testem.
- Jak wycofać konkretną zmianę.

**Język odpowiedzi: polski; kod i nazwy plików mogą być angielskie.**

## 9. Pierwsze polecenie dla agenta

> Przeczytaj `blackberry_bb10_autoroot_ssh_instrukcja.md` i `blackberry_q10_orange_volte_agent_brief.md`. Pracujesz lokalnie w Ubuntu WSL2 na Windows 11. Chcę uruchomić prawdziwe VoLTE na BlackBerry Q10 SQN100-3 w Orange Polska. Zaczynamy od świeżego AutoRoot v4 i czystego `main` repozytorium `bb10root/bb10-volte` zawierającego profil Orange z PR #2. Najpierw sprawdź środowisko, stan repo, komunikację SSH/SCP przez Windows z poziomu WSL oraz bazowe działanie karty Orange bez dodatku. Nie modyfikuj telefonu, nie flashuj i nie uruchamiaj komend destrukcyjnych bez mojej zgody. Przygotuj plan i wykonuj bezpieczne etapy samodzielnie, uzgadniając ryzykowne operacje.

---

**Referencje techniczne:** [AutoRoot v4](https://bb10.root.sx/blog/autoroot-v4) (8/10 dla opisanej procedury), [bb10-volte `main`](https://github.com/bb10root/bb10-volte) (9/10 dla kodu), [PR #2 Orange Polska](https://github.com/bb10root/bb10-volte/pull/2) (9/10 dla zawartości profilu i raportu autora), [Codex CLI w WSL](https://learn.chatgpt.com/docs/windows/wsl) (10/10 dla konfiguracji). Są to źródła pierwotne/techniczne, nie artykuły recenzowane naukowo.
