# Orange Polska VoLTE na BlackBerry Q10 — odtworzenie udanej próby

Data: **7 listopada 2026**, czas lokalny CET. Repozytorium robocze:
`/home/alek/projects/own/bb10-volte` — fork użytkownika.

**Status dokumentu:** zapis pierwszego udanego testu z czasową flagą.
Późniejszą implementację automatycznej obsługi opisuje
[PERSISTENT_IMS_VOICE.md](PERSISTENT_IMS_VOICE.md); jej wdrożenie i test po restarcie
trzeba oceniać na podstawie aktualnego wpisu w `progress.md`.

**Potwierdzony wynik:** użytkownik zgłosił działającą rozmowę; podczas niej
Q10 pozostał w LTE, log IMS pokazał zestawienie SIP oraz wysyłane i odbierane
pakiety RTP. Zadziałało po dodaniu testowej flagi **IMS_VOICE_SUPPORTED**
do obiektu PPS urządzenia. Sam profil Orange i rejestracja IMS nie wystarczały.

**To procedura odtworzenia kontrolowanego testu.** Flaga nie została dodana
do autostartu ani na stałe do menedżera. Po około trzech minutach skrypt
usunął ją automatycznie. Trwałość po restarcie, dłuższa rozmowa i osobne testy
obu kierunków połączeń pozostają do sprawdzenia.

## 1. Warunki wyjściowe

- BlackBerry Q10, według przekazanego opisu **SQN100-3**, AutoRoot v4.
  Oprogramowanie opisane w instrukcji instalacji: BB10 **10.3.3.1463**,
  obraz **10.3.3.2205**. Odczyt `uname` pokazuje QNX 8.0.0/ARM, a nie wersję BB10.
- Root SSH działa przez USB; telefon normalnie się uruchamia.
- Windows 11 + Ubuntu WSL2, Windows OpenSSH 9.5p2.
- Telefon: **169.254.0.1**; interfejs Windows BlackBerry Device:
  **169.254.0.2/30**. To adresy tej instalacji, nie uniwersalna reguła.
- Klucz klienta: **`D:\blackberry\keys\id_rsa4x`**.
  Nie jest to klucz hosta serwera. Nie kopiować go do repozytorium.
- Orange: **MCC 260, MNC 03, Carrier ID 149**.
- VoLTE musi działać na numerze po stronie Orange. Początkowo ta sama karta
  schodziła do 2G również na iPhonie 15 Pro. Użytkownik skontaktował się
  z Orange i później potwierdził działanie VoLTE na iPhonie. Dopiero wtedy
  wykonano opisaną udaną próbę na Q10.

Jeżeli trzeba dopiero uzyskać root/SSH, skorzystać z
[instrukcji AutoRoot i SSH](blackberry_bb10_autoroot_ssh_instrukcja.md).
Ten dokument nie poleca ponownego flashowania i nie opisuje go jako koniecznego
kroku do powtarzania przy każdej próbie VoLTE.

## 2. Sprawdzenie repozytorium i właściwych źródeł

Wszystkie polecenia lokalne poniżej wykonywać w **Ubuntu WSL**, chyba że
oznaczono je jako PowerShell. Zmiany i wyniki zapisywać tylko w tym forku.

```bash
cd /home/alek/projects/own/bb10-volte
git status --short --branch
git remote -v
git rev-parse HEAD
git log -5 --oneline
```

Punkt bazowy udanej próby:

```text
main
baad3eaad0112d5810e5af732703abb2953f37d5
origin: git@github.com:aleksandermpiekarz/bb10-volte.git
```

Jest to merge [upstream PR #2 — Orange Polska](https://github.com/bb10root/bb10-volte/pull/2),
zawierający także zależne zmiany PR #1. Upstream służy jako źródło odniesienia.
Nie wykonywać resetu ani czyszczenia forka w celu „uzyskania czystego main”.

Wymagane pliki:

```text
pkg/data/carrier-149-volte.xml
pkg/data/ir92-149
pkg/data/perms
```

`ir92-149` musi zawierać dokładnie:

```text
@ir92
UsePreconditions:b:true
SMS_Format::3GPP
amr_modeset_list::7
amrwb_modeset_list::2
```

`UsePreconditions` jest typu **bool (`b`)**, nie `n:1`. Profil XML zawiera
osobny APN `ims`, IPv4v6, P-CSCF i signalling bearer przez PCO oraz AMR-WB/AMR.
W `perms` muszą być osobne wpisy:

```text
carrier-149-volte.xml:0:0:644
ir92-149:0:0:644
```

W forku dodano później także profil T-Mobile 240. Nie jest on potrzebny
do Orange. W udanej próbie Orange użyto niezmienionych plików 149 oraz
niezmienionej binarki z bazowego commita; nie scalano PR #5.

## 3. Windows SSH/SCP wywoływany z WSL

W repozytorium przygotowano:

```text
tools/bb-common.sh
tools/bb-ssh
tools/bb-send
tools/bb-fetch
```

Skrypty uruchamiają Windows OpenSSH z następującymi opcjami:

```text
-i D:\blackberry\keys\id_rsa4x
-o IdentitiesOnly=yes
-o BatchMode=yes
-o ConnectTimeout=10
-o StrictHostKeyChecking=yes
-o HostKeyAlgorithms=+ssh-rsa
-o PubkeyAcceptedAlgorithms=+ssh-rsa
-o MACs=+hmac-sha1
```

SCP dodatkowo używa **`-O`** (stary protokół SCP). Ścieżki lokalne są
konwertowane `wslpath -w`; nie należy zakładać, że ZIP z WSL jest na dysku D:.
Klucz hosta musi być wcześniej zweryfikowany i zapisany w Windows `known_hosts`.
Nie wyłączać weryfikacji hosta. Nie potrzeba `sshpass` ani haseł w skryptach.

```bash
bash tools/bb-ssh 'uname -a'
```

Przykładowa działająca odpowiedź:

```text
QNX BLACKBERRY-7B11 8.0.0 ... armle
```

Jeżeli po zmianie SIM lub podłączeniu USB występuje timeout:

```bash
powershell.exe -NoProfile -NonInteractive -Command \
  'Get-NetAdapter | Format-Table Name,Status,InterfaceDescription; Get-NetIPAddress -AddressFamily IPv4 | Format-Table InterfaceAlias,IPAddress; Test-NetConnection 169.254.0.1 -Port 22'
```

Zaczekać na pełne uruchomienie telefonu i odblokować ekran. W tej sesji
zauważono również podejrzany wpis Windows `169.254.0.1/32 → 169.254.0.2`
(własny adres Windows); usunięto wtedy tylko ten konkretny wpis administracyjnym
PowerShell. Nie potwierdzono jednak związku przyczynowego: 8 listopada SSH
działało również z tym wpisem. Nie usuwać tras profilaktycznie — timeout
występował także bez niego. Szczegóły są w [progress.md](progress.md).

## 4. Kompilacja i kontrola paczki

W tej sesji zadziałał lokalny obraz:

```text
ghcr.io/xiazy/blackberry10-toolchain:latest
local image ID:
sha256:60ce49510a37ed92368cca010bbaabf97b12d209b9fab179e8491b266bb2578f
```

ID lokalnego obrazu nie jest digestem manifestu do pobrania z rejestru.

Jeśli budujemy aktualny fork:

```bash
make test
sudo docker run --rm --network none --cap-drop ALL \
  --security-opt no-new-privileges --user "$(id -u):$(id -g)" \
  -v "$PWD":/src -w /src \
  sha256:60ce49510a37ed92368cca010bbaabf97b12d209b9fab179e8491b266bb2578f \
  make -B CC=bb10-cc STRIP=llvm-strip
unzip -t volte.zip
unzip -l volte.zip
file pkg/init/ims_manager pkg/init/devnp-volte-swcrypto.so
sha256sum volte.zip pkg/init/ims_manager pkg/init/devnp-volte-swcrypto.so
```

`make test` dało **36 checks, 0 failures**. Pozostały istniejące ostrzeżenia
GCC o obcięciu MCC/MNC oraz Clang o porównaniu `char` z EOF; nie zmieniano
z tego powodu źródeł. Nie uruchamiać `--stats` jako testu tego ostatniego błędu.

Wynik Makefile jest w **`pkg/init/ims_manager`**. Śledzony w Git plik
`src/ims_manager` nie jest wynikiem tej kompilacji.

Żeby odtworzyć same bazowe źródła bez późniejszego profilu T-Mobile,
można opcjonalnie zbudować archiwum commita w nowym katalogu **wewnątrz forka**:

```bash
mkdir -p artifacts
orange_build_dir=$(mktemp -d "$PWD/artifacts/orange-baad3ea.XXXXXX")
git archive baad3eaad0112d5810e5af732703abb2953f37d5 |
  tar -x -C "$orange_build_dir"
sudo docker run --rm --network none --cap-drop ALL \
  --security-opt no-new-privileges --user "$(id -u):$(id -g)" \
  -v "$orange_build_dir":/src -w /src \
  sha256:60ce49510a37ed92368cca010bbaabf97b12d209b9fab179e8491b266bb2578f \
  make -B CC=bb10-cc STRIP=llvm-strip
unzip -t "$orange_build_dir/volte.zip"
```

To sposób rekonstrukcji z tych samych źródeł, nie dodatkowy krok wykonany
przy udanej rozmowie. Hash ZIP może się różnić przez daty wpisów archiwum.

Zachowana oryginalna paczka z tej sesji:

| Plik | Rozmiar | SHA-256 |
| --- | ---: | --- |
| `artifacts/volte-orange-baad3ea.zip` | 25042 B | `64b82e3557f756bca466c175839f0938f08df021c7950fe92ba750287d8beed2` |
| `pkg/init/ims_manager` | 36200 B | `f4814f5e76cf6dba0d1ae908f6670088403c60c169a0061f85900b0f8e0bd2d3` |
| `pkg/init/devnp-volte-swcrypto.so` | 1844 B | `5b5c3723a1c940523d1905632b5f97df24d77ab1c8a60889705a377672db68d9` |

`artifacts/` jest ignorowane przez Git — tej paczki nie należy oczekiwać
po samym sklonowaniu repozytorium.

## 5. Backup i instalacja na telefonie

Przed pierwszą instalacją zapisano na Q10 katalog
`/var/rootdata/volte-baseline-20261107`, z archiwum `/var/dca/sources` oraz
`/var/ims/carrier` i kopią PPS `cnice/status_persistent`. Po zmianie operatora
wykonywano następne, osobne kopie. Nie nadpisywać tych backupów.

Przykład nowego backupu, uruchamiany z WSL:

```bash
bash tools/bb-ssh '
set -e
backup="/var/rootdata/volte-backup-$(date +%Y%m%d-%H%M%S)"
mkdir "$backup"
chmod 700 "$backup"
if test -d /var/addons/volte; then
    tar -cpf "$backup/config.tar" /var/dca/sources /var/ims/carrier /var/addons/volte
else
    tar -cpf "$backup/config.tar" /var/dca/sources /var/ims/carrier
fi
cp -p /pps/services/cds/cnice/status_persistent "$backup/cnice-status_persistent"
if test -e /pps/services/escreens/config; then
    cp -p /pps/services/escreens/config "$backup/escreens-config"
fi
echo "$backup"
tar -tf "$backup/config.tar"
'
```

To backup konfiguracji dodatku, nie pełna kopia NVRAM/firmware. Dokładne kopie
PPS mogą zawierać ICCID i inne dane abonenta: nie publikować ich ani pełnego slog.
Backup pobrany lokalnie trzymać w ignorowanym `artifacts/` z ograniczonym dostępem.

Przy aktualizacji istniejącej instalacji najpierw zakończyć rozmowę,
sprawdzić `phone_state::idle`, zatrzymać tylko proces dodatku i potwierdzić
jego zakończenie. Nie nadpisywać pracującej binarki. W tej sesji używano
SIGTERM do konkretnego PID odczytanego z `pidin ar`, bez zapamiętywania PID
między restartami.

Instalacja zachowanej paczki Orange:

```bash
set -e
bash tools/bb-send artifacts/volte-orange-baad3ea.zip /var/rootdata/volte-orange-baad3ea.zip
bash tools/bb-fetch /var/rootdata/volte-orange-baad3ea.zip artifacts/orange-upload-check.zip
cmp artifacts/volte-orange-baad3ea.zip artifacts/orange-upload-check.zip
bash tools/bb-ssh '/var/rootdata/installer /var/rootdata/volte-orange-baad3ea.zip'
```

Jeśli używamy nowo zbudowanego ZIP, zmienić wyłącznie lokalne źródło w `bb-send`
i odpowiadające mu `cmp`. Instalator sam nadaje uprawnienia, zaufanie i uruchamia
`/var/addons/volte/init/start.sh`. Oczekiwany wynik: kod 0 i
`Installation completed successfully`.

## 6. Sprawdzenie aktywnego Orange, a nie samej obecności plików

```bash
bash tools/bb-ssh '
echo CARRIER
cat /pps/services/carriermanager/status_private | grep "^true_home_carrier"
echo RADIO
cat /pps/services/cellular/radioctrl/status_cell_public |
    grep -E "^(network_name|network_service|network_technology|serving_network)"
echo PROFILE
cat /pps/services/cds/cnice/status_persistent | grep "^cdsc_xml"
echo APNS
cat /pps/services/cds/cnice/status | grep -E "^(data_services_status|internet|ims):"
echo IR92
cat /pps/services/carrier_config/ims/ir92 | grep -E "^(UsePreconditions|SMS_Format)"
echo IMS
cat /pps/services/ims/cellular/status_private | grep -E "^(state_engine|state_system)"
/var/addons/volte/init/ims_manager -a probe -v
'
```

Warunki osiągnięte przed udaną rozmową:

```text
true_home_carrier: mcc=260, mnc=3, id=149
serving_network: 260/03
network_technology: lte
data_services_status: ON
cdsc_xml: /var/addons/volte/data/carrier-149-volte.xml
internet: active=true, connected=true, apn=internet
ims: active=true, connected=true, apn=ims
UsePreconditions:b:true
state_engine::IMS_ENGSTATE_CONNECTED
state_system::IMS_SYSSTATE_CONNECTED
probe: CONNECTED, INCOMING_READY
```

`BERRY_STOPPED` z osobnego procesu `probe` jest ograniczeniem bazowego kodu;
nie świadczy sam w sobie o zatrzymaniu działającego menedżera.

Po zmianie SIM nowy proces musi ponownie rozpoznać operatora. W naszej udanej
próbie nastąpił normalny start Q10 z Orange; log i PID potwierdziły autostart,
profil 149 oraz właściwy IR.92. Nie trzeba było wtedy reinstalować pakietu.

Gdy działa stary profil, np. po T-Mobile, zatrzymać proces dodatku przez
SIGTERM, potwierdzić jego zakończenie i brak rozmowy, a następnie odświeżyć:

```bash
bash tools/bb-ssh '
cat /pps/services/phone/public/status | grep "^phone_state::idle$" || exit 1
on -d /var/addons/volte/init/ims_manager -a enable -r -v > /var/rootdata/orange-enable.log 2>&1
'
```

**Nie uruchamiać drugiej instancji obok pierwszej.** Odświeżenie może
zrestartować systemowy proces IMS. Samo `ALREADY_CONNECTED` nie gwarantuje,
że wcześniejszy profil został zastąpiony; sprawdzić aktywne `cdsc_xml`.

## 7. Rzeczywista blokada Q10: VoPS=0 i SIP 488

Po naprawie VoLTE przez Orange telefon miał LTE i połączony IMS, a rozmowa
nadal schodziła do 2G. Z logu **14:09:44**:

```text
MT INVITE has Require: precondition
voice: VoPS=0 vdpE=3 ... csAvail=1
VoPS not supported: 1
incoming call is rejected due to domain selection
SIP/2.0 488 Not Acceptable Here
```

Orange dostarczyło rozmowę do IMS, lecz Q10 odrzucił ją przy wyborze domeny
głosowej. To mocniejszy dowód niż sama ikona 4G lub status CONNECTED.

Źródło mechanizmu testowej flagi:
[PR #5](https://github.com/bb10root/bb10-volte/pull/5), plik `src/voice_flag.c`
w rewizji `dd210cd74297a493a31a81eb9d4a661e69e34bf6`.
Nie instalowaliśmy całego PR #5 ani jego automatycznego pobierania profili.

Wartości istotne w tej sesji:

```text
IMS_VOICE_SUPPORTED = 0x02000000 = 33554432
DATA_LTE            = 0x00200000 = 2097152

status_cell_public.network_service = 2097162  (bez testowego bitu)
status_public.network_service      = 2098186  (przed testem)
status_public.network_service      = 35652618 (po OR z 33554432)
```

**Zmieniamy `status_public`, a nie `status_cell_public`.** Maski są różne.
Nigdy nie zastępować maski urządzenia wartością skopiowaną z maski komórki.
Nie wpisywać na stałe 35652618 na innym stanie telefonu: odczytać świeżą maskę,
dodać jeden bit operacją OR, zachować wszystkie pozostałe bity.

## 8. Odtworzenie czasowego testu, który umożliwił rozmowę

Warunki: Orange 149, LTE, IMS CONNECTED, telefon bezczynny, drugi własny telefon
gotowy do zwykłej rozmowy. Test dotyczy zwykłego głosu; nie używać numerów alarmowych.
Nie włączać tego równocześnie z innym procesem wymuszającym tę samą flagę.

Poniższy blok uruchomić w WSL. Wysyła on polecenie do powłoki QNX przez Windows
SSH, utrzymuje bit tylko przy LTE/połączonym IMS i kończy się sam po 150
iteracjach. Z odczytami trwało to około trzech minut. Nie zamykać terminala
w trakcie próby; po zerwaniu SSH osobno zweryfikować sprzątnięcie flagi.
Rozmowę zakończyć przed upływem okna testowego: sprzątanie usuwa flagę także
wtedy, gdy rozmowa nadal trwa.

```bash
set -o pipefail
mkdir -p artifacts
orange_trial=$(cat <<'QNX'
set -e
p=/pps/services/cellular/radioctrl/status_public
mask() {
    cat "$p" | sed 's/^\[n\]//' | sed -n 's/^network_service:n://p'
}
cat /pps/services/carriermanager/status_private |
    grep '^true_home_carrier' | grep '"id":149' >/dev/null
cat /pps/services/phone/public/status | grep '^phone_state::idle$'
original=$(mask)
case "$original" in ''|*[!0-9]*) echo INVALID_INITIAL_MASK; exit 2;; esac
test $((original & 33554432)) -eq 0
cat /pps/services/ims/cellular/status_private |
    grep '^state_system::IMS_SYSSTATE_CONNECTED$'
restore() {
    current=$(mask)
    case "$current" in
        ''|*[!0-9]*) echo RESTORE_NOT_VERIFIED_INVALID_MASK; return 1;;
    esac
    cleared=$((current & ~33554432))
    echo "network_service:n:$cleared" >> "$p"
    echo "TRIAL_RESTORED=$cleared"
}
trap restore EXIT
trap 'exit 130' HUP INT TERM PIPE
echo ORANGE_TRIAL_BEGIN
date
echo "ORIGINAL=$original"
i=0
while test "$i" -lt 150; do
    current=$(mask)
    case "$current" in ''|*[!0-9]*) echo INVALID_MASK_STOPPING; exit 2;; esac
    next=$((current & ~33554432))
    if test $((current & 2097152)) -ne 0 &&
       cat "$p" | grep 'network_technology.*lte' >/dev/null &&
       cat /pps/services/ims/cellular/status_private |
           grep '^state_system::IMS_SYSSTATE_CONNECTED$' >/dev/null; then
        next=$((current | 33554432))
    fi
    if test "$current" -ne "$next"; then
        echo "network_service:n:$next" >> "$p"
        echo "FLAG_UPDATE=$next"
    fi
    if test $((i % 5)) -eq 0; then
        date
        cat /pps/services/cellular/radioctrl/status_cell_public |
            grep -E '^(network_technology|network_service)'
        cat /pps/services/phone/public/status | grep '^phone_state' || true
    fi
    i=$((i+1))
    sleep 1
done
echo ORANGE_TRIAL_END
QNX
)
bash tools/bb-ssh "$orange_trial" | tee artifacts/orange-voice-flag-trial-repeat.txt
```

To ta sama logika, która została wykonana przy udanej próbie; w sesji jej
wyjście zapisywał lokalny Python zamiast `tee`. W QNX `date +%s` zwracało
dosłowne `%s`, dlatego nie opieramy limitu czasu na epoce GNU date.

PPS może zwracać `[n]network_service:n:...`; funkcja `mask()` celowo usuwa
prefiks `[n]`. Wcześniejsza próba z filtrem bez tej obsługi przerwała się
przed poprawnym automatycznym cofnięciem — nie pomijać tej części.

W drugim terminalu WSL można potwierdzić efekt:

```bash
bash tools/bb-ssh '
cat /pps/services/cellular/radioctrl/status_public | grep network_service
slog2info -b ims_rrm | grep -E "bb.domsel:.*VoPS=" | tail -3
'
```

Oczekiwane podczas próby: maska z dodanym bitem i **VoPS=1** w nowym wpisie
logu. `status_cell_public` może nadal pokazywać 2097162 — to normalne dla
tego testu, bo nie jest obiektem, który nadpisujemy.

W tym oknie wykonać zwykłą rozmowę na drugi własny numer, sprawdzić utrzymanie
LTE oraz słyszalność w obu kierunkach. Osobno przetestować połączenie przychodzące.
Nie uznawać samego VoPS=1 ani samej rejestracji IMS za dowód głosu.

## 9. Dowody z udanej próby i stan po niej

```text
14:12:45  uruchomienie próby, maska 2098186 → 35652618, VoPS=1
14:13:46  phone_state=alerting, radio nadal LTE
14:13:53  SIP 200 OK dla INVITE, uruchomienie audio AMR/8000
14:13:58  phone_state=active, radio nadal LTE
14:14:00  BYE; RTP: RECV 70 packets; SENT 310 packets, 13 kb
14:14:03  phone_state=idle, radio nadal LTE
koniec    ORANGE_TRIAL_END, TRIAL_RESTORED=2098186
```

Użytkownik potwierdził: „działa”. Log potwierdza RTP w obie strony i aktywną
rozmowę na LTE. Nie utożsamiać samych liczników RTP z osobnym testem jakości
dźwięku; formalne potwierdzenie obu kierunków rozmów i dłuższego audio pozostaje
kolejnym etapem. Krótka rozmowa nie dowodzi też stabilności po zmianie zasięgu.

Artefakty tej sesji, lokalnie w ignorowanym katalogu:

- `artifacts/orange-voice-flag-trial.txt` — radio i stan telefonu podczas próby.
- `artifacts/orange-success-call-filtered.txt` — SIP/audio/RTP po usunięciu numerów
  i tożsamości abonenta; bez nagrania dźwięku.
- `artifacts/orange-invite-rejection-filtered.txt` — poprzednia odmowa SIP 488.
- `artifacts/volte-orange-baad3ea.zip` — oryginalny pakiet bazowy.

## 10. Cofnięcie testu i dalsze prace

Normalne zakończenie wypisuje **TRIAL_RESTORED**. Następnie sprawdzić:

```bash
bash tools/bb-ssh '
cat /pps/services/cellular/radioctrl/status_public | grep network_service
slog2info -b ims_rrm | grep -E "bb.domsel:.*VoPS=" | tail -3
'
```

W tej sesji test sam przywrócił **2098186**, bez bitu IMS voice. W kolejnej
sesji wartość innych bitów może się różnić. Cofnięcie flagi oznacza, że blokada
VoPS może znów wystąpić; nie przedstawiać tej instalacji jako trwale naprawionej.

Jeżeli trial został przerwany i nie ma potwierdzenia sprzątania, najpierw
upewnić się, że pętla testowa już nie działa i rozmowa jest zakończona.
Poniższe usuwa wyłącznie testowy bit ze świeżej maski. Stosować tylko jako
cofnięcie tego własnego testu, nie do usuwania prawdziwej flagi zgłoszonej przez sieć:

```bash
bash tools/bb-ssh '
set -e
cat /pps/services/phone/public/status | grep "^phone_state::idle$"
p=/pps/services/cellular/radioctrl/status_public
current=$(cat "$p" | sed "s/^\[n\]//" | sed -n "s/^network_service:n://p")
case "$current" in ""|*[!0-9]*) echo INVALID_MASK; exit 2;; esac
cleared=$((current & ~33554432))
echo "network_service:n:$cleared" >> "$p"
cat "$p" | grep network_service
'
```

Nie przywracać całego starego obiektu PPS przez `cp` i nie wpisywać starej
maski podczas innego stanu radia. Nie ma potrzeby resetu Git ani flashowania.

Trwałe rozwiązanie wymaga osobnej implementacji i weryfikacji: ustawianie
flagi tylko przy zarejestrowanym IMS na LTE, usuwanie przy utracie warunków,
bezpieczny start/stop i próby po restarcie oraz utracie zasięgu. Samo utworzenie
pliku `force-ims-voice` niczego nie włączy w używanej bazowej binarce `main` —
obsługę tej nazwy zawiera dopiero inna implementacja z PR #5.

Pełna historia i ograniczenia: [progress.md](progress.md).
