# Postęp: Orange Polska na BlackBerry Q10

## 2026-11-07 — przygotowanie środowiska

### Zakres i stan początkowy

Przeczytano oba dokumenty Markdown w `docs/` i dwa towarzyszące pliki
`Zone.Identifier` (metadane pobrania). Nie znaleziono `AGENTS.md` w repozytorium.
Obowiązuje bieżąca instrukcja użytkownika: jedynym repozytorium roboczym jest
`/home/alek/projects/own/bb10-volte`; historycznych katalogów z instrukcji
nie używano do edycji ani kompilacji.

- Gałąź: `main`, HEAD `baad3eaad0112d5810e5af732703abb2953f37d5`.
- `origin` fetch/push: `git@github.com:aleksandermpiekarz/bb10-volte.git`.
- Początkowo bez zmian śledzonych plików; `docs/` było nieśledzone.
- Nie konfigurowano remote push upstream. Odczyt `git ls-remote`
  `https://github.com/bb10root/bb10-volte.git refs/heads/main` zwrócił ten sam SHA.
- [PR #2](https://github.com/bb10root/bb10-volte/pull/2) scalono jako
  `baad3ea`; commit profilu: `8c0a471ec1227adcdaf6aaa837908717bb0f5f32`.
  Profil Orange, IR.92 i `perms` są identyczne z tym commitem.
  Zależny PR #1 jest również obecny w historii.
- Profil 149 zawiera APN `ims`, IPv4v6, P-CSCF przez PCO i AMR-WB/AMR.
  `ir92-149`: `UsePreconditions:b:true`, `SMS_Format::3GPP`.
  Raport autora PR dotyczy Classic, nie potwierdza działania na Q10.

### Windows SSH/SCP

- WSL2 działa. Windows OpenSSH: 9.5p2 / LibreSSL 3.8.2.
- Windows: interfejs `Ethernet 2`, `BlackBerry Device`, `Up`, `169.254.0.2`.
- Klucz klienta istnieje w `D:\blackberry\keys\id_rsa4x`; nie odczytywano
  jego zawartości, nie kopiowano go, nie zmieniano ACL.
- SSH `root@169.254.0.1` działa z `StrictHostKeyChecking=yes`,
  `BatchMode=yes` i istniejącym Windows `known_hosts`. Nie dodawano nowego
  zaufania ani nie pomijano weryfikacji klucza hosta.
- Odpowiedź: QNX 8.0.0, `BLACKBERRY-7B11`, `armle`; użytkownik `root`.
  Sam `uname` nie potwierdza handlowego numeru wersji BB10 ani wariantu Q10.
- `/var/rootdata` istnieje; `/var/addons` i `/var/addons/volte` nie istnieją.
- Dodano `tools/bb-ssh`, `tools/bb-send`, `tools/bb-fetch` oraz wspólne opcje
  w `tools/bb-common.sh`. Szczegóły i użycie: [environment.md](environment.md).
- Test rzeczywisty: wysłano wyłącznie 67-bajtowy `pkg/MANIFEST` do
  `/var/rootdata/bb10-volte-transport-test.txt`, pobrano do
  `artifacts/scp roundtrip.txt`. `cmp` i SHA-256 potwierdziły zgodność.
  Test objął lokalną ścieżkę UNC WSL oraz spację w nazwie pliku.
  Plik testowy na telefonie pozostawiono; nie jest pakietem instalacyjnym.
- `bash -n` zaakceptował wszystkie cztery skrypty. Niebezpieczna ścieżka zdalna
  zawierająca średnik została odrzucona lokalnie przed wywołaniem SCP.

### Kompilacja i kontrola artefaktów

- Natywne narzędzia: make, cc/gcc, libxml2 2.9.14, zip/unzip, wslpath.
- `make test`: **36 checks, 0 failures**. Oczekiwany test brakującego XML
  wypisuje ostrzeżenie I/O dla `/nonexistent/carrierIds.xml`.
- GCC zgłosił dwa ostrzeżenia o możliwym obcięciu MCC/MNC w
  `src/ims_carrier_config.c:74` i `:78`. Nie poprawiano kodu bazowego.
- Docker wymaga `sudo`; nie zmieniano grup ani uprawnień gniazda Docker.
  Lokalny obraz `ghcr.io/xiazy/blackberry10-toolchain:latest` ma ID
  `sha256:60ce49510a37ed92368cca010bbaabf97b12d209b9fab179e8491b266bb2578f`.
- Pełne `make -B CC=bb10-cc STRIP=llvm-strip` w kontenerze zakończyło się
  kodem 0. Kontener miał wyłączoną sieć, odjęte capabilities,
  `no-new-privileges` i UID/GID 1000:1000; montowany był tylko ten fork.
- Clang ostrzegł w `src/ims_manager.c:123`, że porównanie `char` z EOF
  jest zawsze prawdziwe na tej platformie. To istniejący problem kodu
  (odczyt statystyk); wymaga osobnego uzasadnionego patcha i testu.
- `unzip -t volte.zip`: brak błędów; 14 wpisów, w tym MANIFEST, data/, init/.
  Pliki Orange w ZIP porównano bajtowo ze źródłami; XML jest poprawny składniowo,
  wpisy uprawnień obu plików mają `0:0:644`. Brak końcowego LF w `perms`
  pozostawiono zgodnie z upstream; samo to nie rozstrzyga zachowania instalatora.
- Binarka: ARM EABI5, dynamiczna, interpreter `/usr/lib/ldqnx.so.2`, stripped.
  Biblioteka: ARM EABI5, shared object, stripped.

| Artefakt | Bajty | SHA-256 |
| --- | ---: | --- |
| `volte.zip` | 25042 | `64b82e3557f756bca466c175839f0938f08df021c7950fe92ba750287d8beed2` |
| `pkg/init/ims_manager` | 36200 | `f4814f5e76cf6dba0d1ae908f6670088403c60c169a0061f85900b0f8e0bd2d3` |
| `pkg/init/devnp-volte-swcrypto.so` | 1844 | `5b5c3723a1c940523d1905632b5f97df24d77ab1c8a60889705a377672db68d9` |

Nie używano śledzonego `src/ims_manager` jako wyniku kompilacji.
Źródła `src/`, `pkg/` i `Makefile` pozostają bez zmian śledzonych plików.
Artefakty i katalog `artifacts/` dopisano do `.gitignore`.

### Bazowe odczyty telefonu

Wyłącznie odczyty wybranych pól PPS oraz listy procesów, bez uruchamiania
`ims_manager`. Nie zapisywano pełnych prywatnych PPS ani identyfikatorów abonenta.

- `true_home_carrier`: MCC **260**, MNC **3**, ID **149**;
  profil systemowy `/etc/carrier/config/149.xml`.
- `uicc_state`: **READY**, obecność SIM **YES**.
- `data_service_mode`: **ON**.
- `network_technology`: **umts** w statusie publicznym i komórki.
- `network_service`: **1**; nie przypisano tej wartości niezweryfikowanej interpretacji.
- Działa systemowy `/radio/bin/ims_rrm`; nie zaobserwowano procesu dodatku
  `ims_manager`. Systemowy proces IMS nie jest dowodem rejestracji IMS.
- Ograniczony filtr `/pps/services/ims/status_public` nie zwrócił wybranych pól;
  to nie wystarcza do oceny rejestracji IMS.

**Nie potwierdzono:** rejestracji w Orange, dostępu do internetu LTE,
SMS, zwykłych rozmów ani działania VoLTE. Dane `ON` nie dowodzą transmisji.
Brak `/var/addons` nie dowodzi samodzielnie fabrycznie czystego systemu.

### Następny etap i wycofanie

Najpierw ustalić przez dalsze odczyty stan rejestracji i aktualny tryb sieci;
wyjaśnić UMTS i potwierdzić bazowe LTE Orange. Ręczne SMS i połączenia z drugim
własnym numerem wymagają udziału i zgody użytkownika. Przed instalacją dodatku
potrzebny jest backup konfiguracji i osobna zgoda. Nie instalowano ZIP,
nie restartowano urządzenia/modemu, nie zmieniano usług, PPS ani uprawnień.
Nie wykonano flashowania, resetu Git, czyszczenia repo ani push.

Zmiany przygotowania obejmują tylko narzędzia, dokumentację i ignorowanie
artefaktów; nie powstał commit. Można je wycofać przez usunięcie wyłącznie
nowo dodanych narzędzi/dokumentów i cofnięcie dopisanych reguł `.gitignore`,
zachowując oba dostarczone dokumenty i ich metadane. Nie wymaga to zmiany telefonu.

## 2026-11-07, 12:31 CET — instalacja i pierwsza diagnostyka

### Autoryzacja i backup

Użytkownik rozszerzył zgodę na przesyłanie plików i odwracalne zmiany, następnie
polecił „działaj, instaluj, sprawdzaj”. Instalacja dodatku jest teraz autoryzowana;
wcześniejsze ograniczenie dotyczące osobnej zgody zostało tym zastąpione.
Flashowanie i działania grożące utratą danych nadal są poza zakresem.

Przed instalacją utworzono na telefonie katalog
`/var/rootdata/volte-baseline-20261107` (tryb 700):

- `config.tar`: `/var/dca/sources` oraz pusty `/var/ims/carrier`, z metadanymi.
- `cnice-status_persistent`: dokładna kopia wyjściowego obiektu PPS, przechowywana
  na telefonie jako materiał do przywracania; zawiera dane abonenta i nie jest publikowana.
- `/pps/services/escreens/config` nie istniał. Próba jego skopiowania zwróciła
  błąd; odnotowano brak obiektu, zamiast przedstawiać go jako wykonany backup.

`config.tar` pobrano do ignorowanego `artifacts/baseline-20261107/config.tar`
(lokalny katalog 700, plik 600) i sprawdzono czytelność archiwum.
SHA-256: `ed033c9fdefd0b8e5f96c383aef5bfef67986b48e74f75d48429f17efc10c5ab`.
To kopia konfiguracji związanej z dodatkiem, nie pełny backup telefonu/NVRAM.

### Instalacja i dowody

1. Przesłano niezmieniony `volte.zip` jako `/var/rootdata/volte-baad3ea.zip`.
   Pobrano go ponownie i `cmp` potwierdził identyczność przed instalacją.
2. Uruchomiono `/var/rootdata/installer /var/rootdata/volte-baad3ea.zip`.
   Instalator zakończył się **kodem 0**, utworzył `/var/addons/volte`,
   zastosował `perms`, nadał zaufanie binarce/bibliotece i uruchomił `start.sh`.
3. Proces: PID **22524130**, `/var/addons/volte/init/ims_manager -a enable`.
   Start podczas instalacji potwierdzony; autostart po restarcie nie był testowany.
4. Pobrano zainstalowane `ims_manager`, `devnp-volte-swcrypto.so` oraz `ir92-149`.
   Wszystkie są bajtowo identyczne z lokalnymi wynikami/źródłem. Hash binarki:
   `f4814f5e76cf6dba0d1ae908f6670088403c60c169a0061f85900b0f8e0bd2d3`.
5. `/var/addons/volte/init/ims_manager --self-test`: **7 przypadków PPS przeszło**.
6. `-a probe -v`: **carrier_id=149, mcc=260, mnc=03**, `NOT_CONNECTED`,
   `INCOMING_NOT_READY`. W tej sesji odczyt chronionego `carrierIds.xml` działa;
   nie odtworzono wcześniejszego problemu `carrier_id=-1`.
7. Slog dodatku: **WAITING_FOR_LTE** od 12:31:22. Proces śpi, czekając na LTE.
   Systemowy `ims_rrm` zachował dotychczasowy PID 9572473.

Kod bazowy w `ready_check()` czeka na LTE przed włączeniem providera i zapisem
DCA. IR.92 na telefonie nadal zawiera tylko `@ir92`, a `cdsc_xml` pozostaje pusty.
Archiwum DCA/IMS wykonane po instalacji porównano per plik z backupem:
**brak dodanych/usuniętych plików, brak zmian treści i uprawnień/właścicieli**.
Zmieniły się tylko mtime trzech symlinków `common`, `generic`, `modem_generic`;
dlatego całe archiwa mają różne hashe, mimo niezmienionej konfiguracji.

### Rejestracja komórkowa — obecna przeszkoda

- SIM: gotowa, HPLMN i IMSI HPLMN **260/03**.
- Wybór sieci `AUTO`; technologie GSM/UMTS/LTE są już włączone.
- Radio: **UMTS**, serving network **260/01**, nazwa sieci pusta,
  RSSI **-63 dBm**, 5 kresek. Siła sygnału nie potwierdza dostępu do usług.
- `data_service_mode=ON`, ale **data_services_status=OFF**.
- `ims` i `internet`: `connectable=false`, `active=false`.
- Wynik wcześniejszego skanu zawiera Orange 260/03 GSM i LTE z `availability=18`;
  znaczenia tego bitmasku nie potwierdzono, więc nie uznano go za dowód blokady.
- Log `qct_rrm` z **11:21:14**, sprzed instalacji dodatku:
  `ss_networkRejectIndicationQcci: domain 0x3 :: cause 0x6 for radio 0x1f`,
  następnie `UIM_APP_STATE_ILLEGAL` i `SIM ILLEGAL`.
- To historyczny, bezpośredni dowód odrzucenia rejestracji przez sieć.
  Nie przypisano go jednoznacznie do PLMN ani nie uznano za dowód blokady IMEI.
  [ETSI TS 136 523-1, §9.2.1.1.10](https://www.etsi.org/deliver/etsi_ts/136500_136599/13652301/18.07.00_60/ts_13652301v180700p.pdf)
  potwierdza znaczenie przyczyny NAS #6 jako `Illegal ME` i możliwość pozostania
  USIM w stanie nieważnym do wyłączenia/włączenia urządzenia lub ponownego
  włożenia UICC. Zastosowanie tej interpretacji do tego konkretnego logu
  Qualcomm jest hipotezą; brak pełnego komunikatu NAS i mapowania domeny.

Logi zapisano wyłącznie po odfiltrowaniu tożsamości i długich identyfikatorów:
`artifacts/network-diagnostic-filtered.txt`,
`artifacts/registration-rejects-filtered.txt`. Nie zapisywano pełnego slog.
Nie modyfikowano SIM, IMEI, NVRAM ani trybu sieci; nie wykonywano rozmów,
restartu ani flashowania.

### Kolejny test i cofnięcie

Potrzebne potwierdzenie użytkownika, co pokazuje ekran i czy SIM działa na LTE
w innym telefonie. Najbardziej rozstrzygający test bazowy to porównanie tej SIM
w sprawnym telefonie oraz innej działającej SIM w Q10. Ewentualny pojedynczy
restart/ponowna inicjalizacja SIM może sprawdzić, czy stan po odrzuceniu jest
utrwalony; trzeba obserwować świeże odrzucenia, nie tylko stare wpisy.
VoLTE i głosu **nie potwierdzono**.

Cofnięcie samej obecnej instalacji, dopóki dodatek nadal czeka na LTE:

1. Zweryfikować aktualny PID i wysłać SIGTERM tylko do `ims_manager` dodatku.
2. Przenieść `/var/addons/volte` poza katalog skanowany przez AutoRoot, np.
   do nieistniejącego jeszcze `/var/rootdata/volte-disabled-20261107`.
   Zachowuje to pliki i wyłącza ich wykrywanie przy kolejnym starcie.
3. Potwierdzić brak procesu i niezmienione konfiguracje DCA/IMS.

Nie wykonano cofnięcia: dodatek pozostał zainstalowany i czeka na LTE.
Jeżeli zacznie stosować konfigurację po pojawieniu się LTE, powyższe dwa kroki
nie cofną DCA/IMS. Wtedy należy porównać pliki z backupem, przywrócić zmienione
pliki oraz właściwe atrybuty PPS i odtworzyć stan usług. Nie należy w ciemno
kopiować całego zapisu statusu PPS z powrotem do obiektu.

Nie zmieniano źródeł ani profilu Orange. Nie powstał nowy commit;
lokalna tożsamość Git (`user.name`, `user.email`) nie jest skonfigurowana.

### Obserwacja użytkownika po instalacji

Użytkownik potwierdził, że Q10 pokazuje **tylko połączenia alarmowe**.
Ta sama karta Orange w iPhonie uzyskuje LTE, ale według relacji „po rozmowie”
przełącza się na **E**. Do doprecyzowania: czy przejście następuje już podczas
zestawiania/trwania rozmowy, czy dopiero po zakończeniu, oraz model iPhone'a.
Jeśli rozmowa odbywa się przez 2G, VoLTE w tej próbie nie działa; nie ustalono,
czy powodem są ustawienia/zgodność iPhone'a, aktywacja usługi na numerze czy sieć.
To nie dowodzi uszkodzenia SIM ani nie wyjaśnia samodzielnie odrzucenia Q10.
Najpierw warto potwierdzić działające VoLTE tej karty w zgodnym iPhonie.
Źródło: [Orange — VoLTE](https://www.orange.pl/poradnik/volte-w-telefonie-co-to-jest-i-czy-warto-z-niego-korzystac/).

Uściślenie: **iPhone 15 Pro**, 5G wyłączone; przed rozmową LTE, podczas rozmowy
znika oznaczenie transmisji danych, po rozmowie pojawia się E. Jest to mocna
poszlaka przejścia na głos 2G, ale trzeba wykluczyć wpływ drugiej linii Dual SIM
(czy Orange obsługuje zarówno dane, jak i badaną rozmowę). Wyłączenie 5G nie
wyklucza VoLTE. Kolejne kroki po stronie iPhone'a: sprawdzić tę samą linię
Orange dla głosu i danych, aktualizację ustawień operatora oraz aktywację
VoLTE/IMS na numerze u Orange. Nie stwierdzono jeszcze przyczyny awarii.

Uwaga interpretacyjna: ikony G/E dopiero po rozłączeniu nie są samodzielnym
dowodem technologii rozmowy. Zaproponowano kontrolę podczas trwania połączenia:
wyłączone Wi-Fi i Wi-Fi Calling, jedna aktywna linia Orange dla głosu i danych,
podgląd technologii w Centrum sterowania i próba otwarcia nowej strony.
Działające dane LTE podczas rozmowy w tych warunkach silnie potwierdzają VoLTE;
brak transmisji sam w sobie nie rozstrzyga przyczyny. Wynik testu nieznany.

Użytkownik następnie potwierdził brak internetu podczas rozmowy oraz brak
przełącznika VoLTE na iPhonie. Przegląd zgłoszeń:

- [Nasz Orange, 2021 — brak przełącznika na iPhone 12 Pro/13](https://nasz.orange.pl/t5/Pomys%C5%82y/Prze%C5%82%C4%85cznik-volte-w-iphone-12-pro/idi-p/285734):
  użytkownicy mieli działające VoLTE mimo braku przełącznika. Sam brak opcji
  nie oznacza braku obsługi; problem nie pojawił się dopiero z iPhone 15.
- [Relacja użytkownika iPhone 15 Pro Max, Vodafone](https://www.reddit.com/r/ios/comments/16hho39/cellular_data_turns_off_during_calls/):
  pojedyncza SIM, dane znikają podczas rozmowy i wracają po niej.
  Brak potwierdzonego rozwiązania w tym wątku; to relacja, nie diagnoza.
- [Orange, odpowiedź moderatora zaktualizowana 27.05.2026](https://nasz.orange.pl/t5/Sie%C4%87-i-dost%C4%99pno%C5%9B%C4%87-us%C5%82ug/VoLTE-i-WiFi-Calling-ju%C5%BC-na-iPhonie/m-p/25752):
  sprawdzić status VoLTE/WiFi Calling na koncie, a gdy aktywne i nie działa,
  zgłosić problem do BOK.

Nie znaleziono potwierdzenia powszechnej awarii iPhone 15 Pro o ustalonej
przyczynie odpowiadającej dokładnie tej sytuacji. Rozstrzygający test porównawczy:
inna karta z potwierdzonym VoLTE w tym samym iPhonie; w razie sukcesu podejrzenie
przesuwa się na numer/SIM/konfigurację Orange, a nie brak suwaka w iOS.

## 2026-11-07, 13:45–13:50 CET — przejście na T-Mobile Polska

### Zmiana celu i komunikacja

Użytkownik poinformował, że Orange potwierdziło problem VoLTE i zapowiedziało
naprawę do 7 dni. To informacja od użytkownika po kontakcie z operatorem,
nie niezależnie zweryfikowany termin. Orange odkładamy; użytkownik włożył
działającą kartę T-Mobile i polecił zmienić profil. To zastępuje wcześniejsze
ograniczenie z instrukcji Orange dotyczące niedodawania profili T-Mobile.

Początkowo SSH odpowiadało niestabilnie. Windows miał trasę
`169.254.0.1/32`, interface 9, next hop `169.254.0.2`, metric 1, wskazującą
własny adres Windows. Po odmowie `Access denied` usunięto wyłącznie ten wpis
przez PowerShell podniesiony oknem UAC; polecenie zakończyło się kodem 0.
Późniejszy odczyt nie znalazł tej trasy, a SSH/SCP działały. Przyczyna wszystkich
wcześniejszych timeoutów nie jest udowodniona. Odtworzenie dokładnie tego wpisu,
gdyby okazało się potrzebne, wymaga administracyjnego PowerShell:
`New-NetRoute -DestinationPrefix '169.254.0.1/32' -InterfaceIndex 9 -NextHop '169.254.0.2' -RouteMetric 1 -PolicyStore ActiveStore`.
Indeks interfejsu trzeba ponownie sprawdzić; przy poprawnie działającym SSH nie
ma powodu odtwarzać podejrzanej trasy. Nie zmieniano IP interfejsów ani firewalla.

### Diagnoza i profil

- SIM faktycznie rozpoznana jako **260/02, carrier ID 240**.
- Radio: **LTE**, serving network **260/02**, dane **ON**.
- Stary dodatek po starcie z tą SIM nie miał profilu 240 i wybrał zastępczo
  **carrier-36-volte.xml**. Odczyt `cdsc_xml` potwierdził ten błąd konfiguracji.
  Internet używał APN `3gnet`, IMS `ims`. Zatrzymano ten proces (SIGTERM).
- Przed zmianą wykonano `/var/rootdata/volte-tmobile-baseline-20261107/`
  z archiwum `config-and-addon.tar` (DCA sources, IMS carrier, cały dodatek)
  i kopią `cnice-status_persistent`. Skopiowano też escreens/config, jeśli istniał.
  Ten backup zawiera już stan po automatycznym zastosowaniu profilu 36;
  nie należy traktować go jako fabrycznego, prawidłowego profilu T-Mobile.
- Archiwum pobrano do ignorowanego `artifacts/tmobile-baseline-config-and-addon.tar`,
  sprawdzono listę plików i ustawiono lokalnie tryb 600. SHA-256:
  `05b5a66d2af789d66b9db5e87d7d3753de620d004e9e1ed0a9677a126c738365`.
- Pobrano fabryczny `/etc/carrier/config/240.xml` z podłączonego Q10.
  SHA-256 źródła: `57482932791c973c1b13dc4fcdf9f7ea78ee8760ff1ca81e93b236a6877d0f82`.
  Nie używano historycznego pliku z Windows; przegląd wykazał w nim między innymi
  zdublowane `IMSConfiguration`, więc nie skopiowano go do nowej paczki.
- Dodano `pkg/data/carrier-240-volte.xml`: fabryczny profil, w którym alias
  `ims → internet` zastąpiono pojedynczą usługą `ims` (APN `ims`, IPv4v6,
  P-CSCF i signalling bearer przez PCO, AMR-WB/AMR) oraz włączono ikonę VoLTE.
- Semantyczne porównanie XML potwierdziło zachowanie wszystkich pozostałych
  ustawień, w tym `internet`, `mms` i `lte_admin`. Wpis uprawnień: `0:0:644`.
- Nie dodawano `ir92-240`: pozostawiono domyślny IR.92 z bazowej implementacji.
  Nie przenoszono ustawień preconditions Orange ani nie wymuszano głosu IMS.
- Profil i IR.92 Orange zachowano. Źródeł C, Makefile ani start.sh nie zmieniano.

### Paczka, instalacja i kontrola

- `make test`: **36 checks, 0 failures**; te same dwa ostrzeżenia GCC co poprzednio.
- Spakowano istniejące zweryfikowane binarki ARM/QNX (`make package`), bez
  ponownej kompilacji kodu C. `unzip -t`: **15 wpisów, brak błędów**.
- `volte.zip`: **26239 bajtów**, SHA-256
  `189bafd1275947f9204aa5d84e2521558198d57e60b5549796072435d217d719`.
  Poprzednia paczka Orange: `artifacts/volte-orange-baad3ea.zip`.
- Przesłano `/var/rootdata/volte-tmobile-240.zip`, pobrano ponownie i porównano
  `cmp` przed instalacją. Instalator AutoRoot zakończył się **kodem 0**.
- Instalator uruchomił proces, który zgłosił `ALREADY_CONNECTED`, ale zachował
  profil 36. Dlatego po potwierdzeniu `phone_state=idle` zatrzymano tylko proces
  dodatku i uruchomiono `ims_manager -a enable -r -v` przez `on -d`.
- Log potwierdził profil **carrier-240-volte.xml**, ponowne uruchomienie `ims_rrm`
  oraz **CONNECTED** o 13:49:48. Menedżer PID 20226162, IMS PID 20271106.
- Aktywny `cdsc_xml`: **/var/addons/volte/data/carrier-240-volte.xml**.
- APN **internet** i **ims**: `active=true`, `connected=true`; dane ON, radio LTE.
- `probe -v`: MCC 260/MNC 02/ID 240, **CONNECTED**, **INCOMING_READY**.
  `BERRY_STOPPED` dotyczy lokalnego stanu procesu probe, nie rozstrzyga stanu
  osobnego działającego procesu monitora.
- Na telefonie brak `imsconfig` dla 260/02, więc bazowy kod pominął modyfikację
  XML `AllowVoLteProvisioning`. Ogólny komunikat programu o zastosowaniu
  provisioning nie jest dowodem zmiany nieistniejącego XML.
- Pobrany z telefonu profil jest bajtowo identyczny z lokalnym. SHA-256:
  `9b22b848491934c0c100b15ea06b9fd665437bf4d062475065e3165799948f87`.
- `network_service=2097162` zapisano jako surowy odczyt, bez wymuszania bitów.

Poproszono użytkownika o zwykłą rozmowę na drugi własny numer i potwierdzenie
LTE oraz dźwięku w obu kierunkach. Uruchomiono krótką obserwację radio/phone/IMS
do `artifacts/tmobile-state-observation.txt`, bez numerów i tożsamości abonenta.
**Rozmowy VoLTE, przychodzące i wychodzące, nie są jeszcze potwierdzone.**

Obserwacja 13:50:38–13:51:25: **10/10 próbek LTE**, `phone_state=idle`,
`IMS_ENGSTATE_CONNECTED` i `IMS_SYSSTATE_CONNECTED`. Nie zaobserwowano rozmowy
w tym oknie; pomiar dotyczy stabilności w spoczynku. Ponowny test dekodera PPS
na zainstalowanej binarce: **7/7**.

Powrót do Orange po naprawie operatora: zachowano profil 149 i jego IR.92,
więc po zmianie SIM konieczne jest ponowne wykrycie operatora przez nowy proces
dodatku i kontrolowane odświeżenie konfiguracji. Nie wystarczy sama zamiana SIM
w pracującym menedżerze, który pamięta poprzedni carrier ID. Przed odświeżeniem
sprawdzić brak rozmowy i wykonać aktualny backup. Stara paczka/backup pozwalają
cofnąć pliki dodatku, ale nie cofają samodzielnie zastosowanych ustawień PPS/DCA.

### Próba rozmowy T-Mobile — zejście do 2G

Użytkownik potwierdził wykonanie próby połączenia i przejście Q10 z 4G na małe
`e`. Kierunku połączenia nie uznano za rozstrzygnięty na podstawie samego opisu.
To wynik negatywny dla VoLTE w tej próbie, mimo wcześniejszego CONNECTED.

Zdarzenia dodatku: 13:51:31 `LTE_LOST`, następnie automatyczne zatrzymanie
`ims_rrm`; po 15 s bazowy menedżer użył SIGKILL, ponieważ proces nie zniknął.
13:51:49 LTE wróciło; próba odtworzenia IMS 1/6 zakończyła się CONNECTED
o 13:51:52. Użytkownik potwierdził, że w tym czasie próbował dzwonić — nie należy
przedstawiać tej utraty LTE jako przypadkowego problemu zasięgu.

Odczyty po rozmowie:

- Poprawna rejestracja IMS potwierdzona także logiem `Registration successful`
  oraz przejściem do `Registered`, nie tylko odczytem APN.
- `imsEnable=true`, `volteEnable=true`; przełączniki UI mogą być wyłączone
  bez wyłączenia samej funkcji (`enableVolteSwitch=false`).
- Efektywne DCA zawiera usługę głosową IMS dla LTE i
  `voice_domain_preference_EUTRAN=3`.
- Kluczowy log `ims_rrm` po ponownej rejestracji:
  `voice: VoPS=0 vdpE=3 vdpU=0 ... csAvail=1`, następnie
  **`VoPS not supported: 1`**. To bezpośredni ślad blokady wyboru domeny IMS.
- `status_cell_public.network_service=2097162` (`0x20000a`) nie ma bitu
  `0x2000000`; maska `status_public` jest odrębna (odczyt 2098186), więc nie
  wolno w ciemno zastępować jej wartością z `status_cell_public`.
- Przejrzano [PR #5](https://github.com/bb10root/bb10-volte/pull/5) i źródło
  `voice_flag.c` w rewizji `dd210cd74297a493a31a81eb9d4a661e69e34bf6`.
  Opisuje bit `IMS_VOICE_SUPPORTED=0x2000000` oraz odmowę rozmów IMS bez niego.
  Nie scalono PR ani nie uruchomiono jego automatycznego pobierania profili,
  podmian plików systemowych i trwałego wymuszania bitu.

Filtrowany log próby: `artifacts/tmobile-call-fallback-filtered.txt`.

### Test izolujący flagę VoPS

W ramach zgody na odwracalne zmiany wykonano krótki test bitu
`0x2000000` w **status_public**, po sprawdzeniu bezczynności telefonu
i połączonego IMS. Pozostałych bitów maski nie zmieniano.

- Pierwsza próba przerwała się bez zapisu: zabezpieczenie oczekiwało maski
  komórki, podczas gdy obiekt urządzenia ma inną maskę. Warunek poprawiono,
  używając świeżo odczytanej wartości i bitowego OR.
- Oryginał: **2098186**. Po dodaniu bitu o **13:54:38** log `ims_rrm`
  potwierdził **VoPS=1**. To bezpośredni dowód wpływu tego pola na wybór domeny.
- Odczyt PPS otrzymał prefiks `[n]network_service`; pierwszy filtr nie obsłużył
  go i przerwał polecenie wcześniej niż planowano. W efekcie automatyczne
  przywrócenie w tej próbie nie zapisało pola. Wykonano osobne przywrócenie
  oryginalnej maski **2098186**; log z **13:55:16** potwierdził **VoPS=0**.
  Test trwał około 38 s, nie planowane 3 s. Nie wykonano w nim automatycznego
  wybierania numeru ani nie potwierdzono rozmowy.
- Następna ograniczona próba obsługuje prefiks `[n]`, odczytuje aktualną maskę
  przed każdą zmianą, ustawia bit wyłącznie przy LTE i połączonym IMS, a po
  zakończeniu/obsługiwanym sygnale usuwa tylko ten bit. Nie instaluje trwałego
  serwisu ani nowej binarki. Użytkownik został poproszony o zwykłą rozmowę
  między własnymi telefonami w oknie około 90 s.

Wynik obserwacji tej próby: `artifacts/tmobile-voice-flag-trial.txt`.
Samo VoPS=1 uzyskane przez nadpisanie PPS nadal **nie jest dowodem**, że sieć
przyjmuje rozmowy VoLTE; potrzebna jest próba głosu/SIP i sprawdzenie RTP/audio.

### Przerwana próba i powrót do Orange

W próbie czasowej o 13:56:53 ustawiono maskę urządzenia 35652618
(2098186 OR 0x2000000). Log potwierdził VoPS=1. O 13:58:00 radio przeszło
na GSM, telefon był `alerting`, następnie `active`; o 13:58:17 SSH zostało
zerwane (`Connection reset`). W lokalnym logu brak końcowego potwierdzenia
`TRIAL_RESTORED`. Nie wolno twierdzić, że automatyczne sprzątanie zostało
zweryfikowane. Polecenie było ograniczone do 90 iteracji i usuwało testowy bit
przy braku LTE/IMS; nie instalowano trwałego procesu ani pliku wymuszającego.

Użytkownik wyjaśnił, że otrzymał w tym czasie rzeczywistą rozmowę przychodzącą,
również przez 2G, i poprosił o wstrzymanie. Następnie poinformował, że Orange
przywróciło już VoLTE, i podłączył Q10 z tą kartą do nowej próby. Prace wznowiono
z celem powrotu do oryginalnego profilu Orange 149.

Ponownie sprawdzono `artifacts/volte-orange-baad3ea.zip`: brak błędów ZIP,
profil 149, IR.92 i binarka zgodne bajtowo z lokalnymi plikami bazowymi.
Zmiana na telefonie wymaga świeżej identyfikacji karty i ponownego startu
menedżera, ponieważ działający proces może pamiętać carrier ID 240.

Na początku próby po ponownym podłączeniu Windows widział `BlackBerry Device`
Up i IP 169.254.0.2/30, ale sąsiad 169.254.0.1 był `Unreachable` bez adresu MAC,
a SSH zwracało timeout. Błędna trasa /32 nie występowała. Poproszono o
odblokowanie i zakończenie uruchamiania Q10. Nie zastosowano nowej konfiguracji
Orange, dopóki nie odzyskano połączenia; weryfikacja stanu testowej flagi również
pozostaje do wykonania. Nie należy usuwać potencjalnej prawdziwej flagi VoPS
Orange tylko na podstawie wcześniejszego testu T-Mobile.

## 2026-11-07, 14:08 CET — Orange po przywróceniu VoLTE przez operatora

Po odzyskaniu USB potwierdzono aktualny stan telefonu. Q10 uruchomiono ponownie
o **14:03:59** (BootTime), więc proces czasowego testu T-Mobile nie przetrwał.
Maska urządzenia **2098186** i maska komórki **2097162** nie mają bitu
`0x2000000`; nie ma pozostałości wymuszenia VoPS.

- SIM: **Orange 260/03, carrier ID 149**; rejestracja radiowa w **260/03**,
  nazwa operatora **Orange**, technologia **LTE**, dane **ON**.
- Autostart po restarcie jest faktycznie potwierdzony: działa
  `ims_manager -a enable` PID 17711162, `ims_rrm` PID 17817645.
- Menedżer sam wybrał **carrier-149-volte.xml** po starcie z Orange.
  Nie trzeba było przeinstalowywać ani ponownie odświeżać konfiguracji.
- Źródło i efektywny obiekt IR.92 zawierają **UsePreconditions:b:true**,
  `SMS_Format::3GPP`, modesety AMR 7 i AMR-WB 2.
- APN **internet** i **ims**: aktywne i połączone. Stany IMS engine/system:
  **CONNECTED**. `probe`: **CONNECTED**, **INCOMING_READY**, carrier 149.
- Profil i `ir92-149` pobrano ponownie i porównano `cmp` ze źródłami:
  są identyczne z bazowym profilem Orange PR #2.
- Nadal występuje **VoPS=0 / VoPS not supported** w logu `ims_rrm` (14:07:53),
  mimo gotowej sesji IMS. Nie należy utożsamiać jej z działającym głosem.

W tej iteracji nie zmieniano konfiguracji telefonu ani flagi VoPS; potwierdzono
stan wybrany przez autostart. Użytkownik potwierdził działanie VoLTE tej SIM
po stronie Orange na iPhonie, ale głos na Q10 wymaga jeszcze nowej próby.
Następny test: zwykłe połączenie **wychodzące z Q10** na drugi własny numer,
kontrola utrzymania LTE i dźwięku w obu kierunkach. Potem osobno przychodzące.

### Pierwsza rozmowa Orange po naprawie usługi — diagnoza odmowy

Użytkownik zgłosił ponowne przejście do `e`. Log z **14:09:44** potwierdza,
że do Q10 dotarł **MT SIP INVITE** z `Require: precondition`. Telefon odrzucił
go z powodu wyboru domeny: `incoming call is rejected due to domain selection`,
bezpośrednio po `VoPS=0` / `VoPS not supported`, i wysłał
**SIP/2.0 488 Not Acceptable Here**. Nie uznano tego za odmowę po stronie Orange
ani za dowód błędu kodeków. O 14:09:45 dodatek zgłosił LTE_LOST; IMS odtworzono
po powrocie LTE o 14:10:40. Efektywne preconditions pozostało włączone.

Dowody po filtracji tożsamości: `artifacts/orange-first-call-filtered.txt`,
`artifacts/orange-invite-rejection-filtered.txt`.

Na tej podstawie uruchomiono ograniczoną czasowo próbę bitu VoPS na Orange,
po potwierdzeniu carrier 149, braku rozmowy, połączonego IMS i braku testowego
bitu w masce początkowej. 150 iteracji ze snem 1 s (około 3 min z odczytami)
ustawia bit tylko przy LTE i połączonym IMS; przy utracie tych warunków usuwa go.
Zakończenie oraz obsługiwane HUP/INT/TERM/PIPE wywołują usunięcie samego bitu
na podstawie świeżej maski. Test nie zmienia profilu, binarki ani autostartu.

Log na żywo: `artifacts/orange-voice-flag-trial.txt`. Wynik rozmowy i potwierdzenie
końcowego przywrócenia pozostają do odczytania po próbie; nie należy uznawać
VoPS=1 za dowód sukcesu głosu. QNX `date +%s` zwraca dosłowne `%s`, dlatego
ograniczenie czasu używa skończonej liczby iteracji, a nie epoki z GNU date.

## 2026-11-07, 14:13–14:15 CET — udana rozmowa Orange na LTE

Użytkownik potwierdził działanie rozmowy po włączeniu testowej flagi VoPS.
Obserwacja radia potwierdziła LTE podczas `phone_state=alerting` (14:13:46,
14:13:52) i `phone_state=active` (14:13:58). O 14:14:03 telefon był ponownie idle.

Log IMS potwierdza **SIP 200 OK dla INVITE**, uruchomienie audio **AMR/8000**,
odbiór **70 pakietów RTP** i wysłanie **310 pakietów RTP / 13 kb**, a następnie
BYE o 14:14:00. To dowód zestawienia głosu IMS i transmisji mediów w obie strony
na LTE, wsparty relacją użytkownika. Nie zastępuje osobnego potwierdzenia
obu kierunków połączeń, jakości dłuższej rozmowy i działania po restarcie.

Zapis po filtracji tożsamości: `artifacts/orange-success-call-filtered.txt`.
Nie nagrywano dźwięku.

Próba zakończyła się samoczynnie; lokalny log i kod wyjścia 0 potwierdzają:
`ORANGE_TRIAL_END`, **`TRIAL_RESTORED=2098186`**. Nie zainstalowano trwałego
wymuszania VoPS. Powtórzenie działania wymaga kolejnego kontrolowanego testu
albo osobnego wdrożenia trwałej, bezpiecznie wyłączanej obsługi flagi.

Na prośbę użytkownika zapisano pełną procedurę odtworzenia z poleceniami,
backupem, hashami, diagnozą SIP 488, ograniczonym testem i cofnięciem:
[ORANGE_Q10_VOLTE_DZIALAJACA_PROCEDURA.md](ORANGE_Q10_VOLTE_DZIALAJACA_PROCEDURA.md).

## 2026-11-07 — zapis wyników i publikacja forka

Użytkownik polecił zapisać i wypchnąć lokalne zmiany wyłącznie do własnego
repozytorium `aleksandermpiekarz/bb10-volte`, przed następną próbą T-Mobile.
Zakres: dokumentacja dostarczona i opracowana w sesji, narzędzia Windows SSH/SCP,
profil T-Mobile 240 i jego uprawnienia oraz reguły ignorowania artefaktów.
Nie dodajemy ZIP-ów, binarek, backupów, prywatnych logów ani `Zone.Identifier`.

Kontrola przed commitem: `make test` **36 checks, 0 failures**, Bash syntax,
XML profilu 240 i `git diff --check` poprawne. Pozostały opisane wcześniej
ostrzeżenia GCC. Przegląd publikowanych plików nie wykazał przekazanych haseł,
kluczy prywatnych, tokenów ani rzeczywistego ICCID abonenta.

Linux SSH rozpoznaje klucz GitHub, lecz nie ma dostępnego agenta do podpisania
żądania w trybie bezobsługowym. Windows OpenSSH potwierdził uwierzytelnienie
jako `aleksandermpiekarz`; polecenia Git do origin używają go przez
`GIT_SSH_COMMAND`, bez kopiowania kluczy ani zmiany remote.
Wyłącznie w tym repo ustawiono autora Git `aleksandermpiekarz` z publicznym
adresem GitHub noreply `48812006+aleksandermpiekarz@users.noreply.github.com`.

Następna próba T-Mobile będzie wymagała potwierdzenia włożenia SIM 260/02,
ponownego wykrycia carrier ID 240 i kontroli aktywnego profilu. Udany test
Orange nie jest dowodem działania głosu T-Mobile.

## 2026-11-07, 15:33 CET — wdrożenie automatycznej flagi IMS voice

Na wyraźną prośbę użytkownika zaimplementowano trwałą obsługę udanego testu
w menedżerze, zamiast czasowej pętli SSH. Plik `data/force-ims-voice` jest
przełącznikiem, dołączonym do pakietu. Pętla enable co sekundę odczytuje świeżą
maskę urządzenia, LTE/SIM/data i rejestrację IMS; dodaje/usuwa wyłącznie bit
0x02000000. Nie zmieniono profili Orange/T-Mobile ani flag alarmowych.

Zmiany i plan: [PERSISTENT_IMS_VOICE.md](PERSISTENT_IMS_VOICE.md),
[persistent-ims-voice-plan.md](persistent-ims-voice-plan.md).
Obsługa marker OFF przestaje zarządzać bitem po sprzątnięciu. Dodano blokadę
drugiego procesu przed pierwszym zapisem, cleanup na kontrolowany exit,
ponawianie cleanup podczas oczekiwania na usługi, oczekiwanie enable na start
SIM/LTE oraz nieblokujący odczyt i poprawkę wyścigu startu monitora Berry.

Weryfikacja: test najpierw czerwony (brak modułu), potem **41** kontroli polityki
i parsowania. Po niezależnym przeglądzie dodano test blokady rzeczywistych
procesów (czerwony: brak funkcji), potem **49/49**. Pełne testy **36 + 49**, zero
błędów. Przegląd wskazał także ponawianie cleanup po błędach startowego PPS —
poprawiono. Build ARM/QNX i ZIP poprawne. Pozostały wcześniejsze ostrzeżenia
MCC/MNC i `char`/EOF; nie dotyczą nowego modułu. Nie testowano awarii odczytu PPS
ani SIGTERM dokładnie podczas blokującej recovery; SIGKILL/awaria nie gwarantują
sprzątnięcia flagi, a błąd odczytu/zapisu jest raportowany bez wpisywania starej maski.

Przed instalacją utworzono `/var/rootdata/volte-before-persistent-20261107/`
z backupem dodatku/DCA/IMS i dokładnym PPS cnice; pobrane archiwum znajduje się
w ignorowanym `artifacts/backup-before-persistent.tar`, lokalnie tryb 600,
SHA-256 `337171e2dadc98200f12a9917f1b088d154d60ed3ef144de64e0ed60816d054e`.
Stary menedżer 17711162 zakończył się po SIGTERM; potwierdzono brak procesu
przed zastąpieniem plików. Transfer nowej paczki porównano `cmp` w obie strony.
Installer zakończył się kodem 0 o 15:33:15.

| Artefakt | Bajty | SHA-256 |
| --- | ---: | --- |
| `artifacts/volte-persistent-voice-v1.zip` | 27888 | `73532a0eb52f5765feb9fe871ce0f7ed2b4482508420d119340564ef1ae5536d` |
| `pkg/init/ims_manager` | 39044 | `5f58a2258ae21707590f9f85700b55160cde2e953c5a62d2d0697143141e29a2` |

Zainstalowaną binarkę pobrano i porównano bajtowo z buildem. Test PPS na Q10
**7/7**. Log automatycznie zgłosił IMS_VOICE_FLAG ON, maskę 35652618 i VoPS=1.
Test marker OFF: 2098186; ON: 35652618. Drugi `enable`: kod 1, ENABLE_LOCK_FAILED,
maska pozostała włączona. Kontrolowany SIGTERM: brak procesu i maska 2098186;
ponowny start: flaga samoczynnie ON. Wyłączenie markerem przestało ingerować
w kontrolowany bit wpisany później w teście, a następnie ten wpis cofnięto.

Ostateczny menedżer uruchomiono niezależnie od SSH (stdin /dev/null, stdout/stderr
do `manager.log` w katalogu backupu). PID 34091053, Orange, LTE i VoPS=1.
Restart całego telefonu jest następnym uzgodnionym testem; nie zaliczamy go
na podstawie samego restartu procesu. Przed nim BootTime: 14:03:59.

Kod wdrożonej wersji zapisano w **a3faee8** na gałęzi `feat/persistent-ims-voice`.
Po potwierdzeniu bezczynności wykonano `/bin/shutdown -S reboot`. Składnię
sprawdzono uprzednio przez `use shutdown`: na tym BB10 **`-b` oznacza wyłączenie
bez restartu**, więc nie wolno przenosić odmiennych przykładów QNX w ciemno.
Telefon przyjął restart i zamknął SSH po procedurze shutdown.

Przy ponownym pojawieniu się interfejsu Windows odtworzył podejrzaną trasę
169.254.0.1/32 przez własny adres 169.254.0.2 (interface 9). Uruchomiono usunięcie
wyłącznie tego wpisu z podniesieniem UAC, tak jak wcześniej. Proces hosta czeka
na odpowiedź UAC; zwykły PowerShell nie ma uprawnienia Remove-NetRoute.
W tym momencie SSH zwracało `banner exchange ... Connection refused`, więc
**autostart i VoPS po pełnym restarcie nie są jeszcze zweryfikowane**.
Poproszono użytkownika o stan okna UAC i ekranu telefonu. Do czasu potwierdzenia
gałąź z nowym kodem nie zastępuje main; T-Mobile jeszcze nie testowano.

Ostatecznie UAC zostało anulowane (`The operation was canceled by the user`);
nie zmieniono trasy i nie ponawiano żądania podniesienia uprawnień. Kolejny
odczyt SSH nadal zwrócił odmowę podczas wymiany bannera. Wdrożony kod i wyniki
testów sprzed restartu są zapisane i wypchnięte na `origin/feat/persistent-ims-voice`.
Do dalszej weryfikacji potrzebne jest odzyskanie SSH i stan ekranu Q10.

## 2026-11-08 — relacja po restartach, następna próba T-Mobile

Użytkownik poinformował, że pierwszy start Q10 po wdrożeniu utknął na napisie
BlackBerry. Kolejna próba również nie dała oczekiwanego działania; dopiero
następny restart przywrócił działanie i **VoLTE Orange działało po restarcie**.
To potwierdzenie użytkownika, nie pełny zapis diagnostyczny poprzednich startów.
Nie należy uznawać rozruchu za stabilny ani przypisywać przyczyny zawieszenia
samemu menedżerowi bez logów. Oczekiwanie enable działa w tle przez `on -d`,
ale potrzebna jest obserwacja problemu, jeśli się powtórzy. Nie wykonujemy
kolejnych restartów tylko w celu wywołania zawieszenia.

Użytkownik podłączył teraz Q10 z kartą **T-Mobile** i polecił użyć odpowiedniego
profilu, zachowując trwałą flagę. Profil `carrier-240-volte.xml` i marker są już
w źródłach oraz poprzednio zainstalowanej paczce. Najpierw odczytujemy aktualny
carrier ID, aktywny profil, flagę i rejestrację; nie zastępujemy działającej
binarki bez potrzeby. Celem jest normalna rozmowa T-Mobile na LTE z audio,
nie tylko stan CONNECTED. Gałąź robocza: `feat/persistent-ims-voice`.

Odczyt o 11:28–11:29 potwierdził **BootTime 11:12:59**, nowy menedżer PID
14057566 oraz `ims_rrm` PID 14274749. Karta faktycznie **260/02, carrier 240**;
telefon wybrał **carrier-240-volte.xml** bez ręcznego przełączania. Profil
pobrany z telefonu jest identyczny z plikiem w repo (SHA-256
`9b22b848491934c0c100b15ea06b9fd665437bf4d062475065e3165799948f87`), binarka
z wdrożoną wersją (SHA-256
`5f58a2258ae21707590f9f85700b55160cde2e953c5a62d2d0697143141e29a2`).
Nie przeinstalowywano dodatku ani nie modyfikowano obecnego profilu.

T-Mobile: **LTE 260/02**, dane ON, APN `internet` i `ims` aktywne i połączone,
IMS engine/system CONNECTED. Domyślny IR.92: `UsePreconditions:n:0`,
SMS 3GPP i modesety AMR 7 / AMR-WB 2. Nie przenoszono wartości Orange
bez dowodu, że T-Mobile wymaga preconditions.

Marker istnieje, maska urządzenia **35652618**, log `ims_rrm` potwierdza
**VoPS=1**. Startowy log: WAITING_FOR_SERVICES 11:19:36, APPLYING 11:19:53,
CONNECTED i flaga ON 11:20:06. Wystąpiła utrata IMS, OFF i LTE, następnie
automatyczne odtworzenie do CONNECTED/ON o 11:20:55. Nie przypisujemy temu
przyczyny bez dalszych danych. Obecny autostart i działanie flagi po pełnym
uruchomieniu są potwierdzone, ale nie oznacza to niezawodnego rozruchu.

Ważne uściślenie diagnostyki USB: Windows nadal ma trasę 169.254.0.1/32 przez
169.254.0.2, a **SSH/SCP działają**. Wcześniejsze przypisanie jej przyczyny
timeoutów było hipotezą, nie dowodem. Dziś nie zmieniano tras, nie żądano UAC.
Nie należy usuwać wpisu przy działającym połączeniu.

Uruchomiono odczytową obserwację radio/phone/IMS do
`artifacts/tmobile-20261108-call-observation.txt`. Kolejny etap wymaga zwykłej
próby połączenia użytkownika z Q10 na drugi własny numer i potwierdzenia LTE
oraz audio, a następnie osobno połączenia przychodzącego. Głosu T-Mobile
nie potwierdzamy na podstawie samej rejestracji IMS.
