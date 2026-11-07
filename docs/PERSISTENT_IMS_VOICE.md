# Trwała obsługa IMS voice na Q10

Ta wersja menedżera automatycznie odtwarza efekt udanego testu Orange opisany
w [procedurze pierwszej rozmowy](ORANGE_Q10_VOLTE_DZIALAJACA_PROCEDURA.md).
Wdrożenie i wynik kontroli konkretnej binarki są zapisywane w [progress.md](progress.md).

## Mechanizm

Obecność `/var/addons/volte/data/force-ims-voice` włącza eksperymentalne
zarządzanie bitem `IMS_VOICE_SUPPORTED` (0x02000000). Plik jest dołączony
do paczki. Nie jest to stała liczba wpisana do konfiguracji modemu.

W trybie `enable` pętla główna co sekundę:

1. Czyta aktualną maskę `network_service` z **status_public** urządzenia.
2. Dodaje tylko bit IMS voice, gdy jest LTE, gotowa SIM, włączone dane
   i połączony IMS. Parser wymaga również bitu DATA_LTE w tej masce.
3. Zdejmuje zarządzany bit po utracie warunków lub usunięciu pliku przełącznika.

Kontrolowane wyjście procesu również zdejmuje bit. Pozostałe bity są zachowane;
maska z `status_cell_public` nie jest kopiowana do `status_public`. Obsługiwany
jest prefiks `[n]`. Niepełne liczby, przepełnienie, brak pola i błędne dane
nie powodują zapisu przypadkowej maski. Błąd odczytu/zapisu jest raportowany;
w takim przypadku sprzątnięcie nie jest gwarantowane. SIGKILL/awaria procesu
również nie uruchomią cleanup — nie należy obiecywać ich obsługi.

Blokada `ims-manager.lock` zapobiega startowi drugiej instancji `enable` przed
jakimkolwiek zapisem flagi; nie usuwać pliku blokady, gdy działa menedżer.
Po usunięciu markera i udanym cleanup proces przestaje zarządzać tym bitem,
więc nie kasuje późniejszych, prawdziwych wskazań modemu.

Synchronizacja nie zależy od wątku odtwarzającego IMS, który może czekać na
zatrzymanie usługi. `check`, `probe`, `--self-test` i `--help` nie uruchamiają
tej funkcji. Nie zmieniono profilu Orange, ustawień alarmowych ani trybu sieci.

Start `enable` czeka na gotowość usług/SIM/LTE i brak rozmowy. Naprawiono też
wyścig startu monitora Berry i jego blokujący odczyt, żeby zakończenie procesu
nie wymagało następnego zdarzenia IMS. Sygnały TERM/INT tylko zgłaszają zatrzymanie;
sprzątanie odbywa się poza handlerem sygnału.

## Instalacja i kontrola

Zbudować paczkę zgodnie z [environment.md](environment.md), wykonać backup,
zakończyć rozmowę, zatrzymać poprzedni menedżer i potwierdzić jego zakończenie
przed aktualizacją. Installer sam uruchamia nową wersję. Nie uruchamiać
równolegle dawnej pętli testowej i nowego menedżera.

```bash
make test
bash tools/bb-ssh 'pidin ar | grep ims_manager'
bash tools/bb-ssh 'cat /pps/services/cellular/radioctrl/status_public | grep network_service'
bash tools/bb-ssh 'slog2info -b bb10_volte | grep IMS_VOICE_FLAG | tail -10'
bash tools/bb-ssh 'slog2info -b ims_rrm | grep -E "bb.domsel:.*VoPS=" | tail -3'
```

Log `IMS_VOICE_FLAG: ON` i `VoPS=1` potwierdzają działanie mechanizmu,
ale głos wymaga normalnej próby rozmowy, kontroli LTE i audio w obu kierunkach.
Obecny plik marker i start.sh mają przetrwać restart; to należy sprawdzić
na telefonie przez zmianę BootTime/PID i ponowną rozmowę, nie przez samą
obecność plików.

## Wyłączenie bez odinstalowywania

W WSL, bez trwającej rozmowy:

```bash
bash tools/bb-ssh 'test ! -e /var/addons/volte/data/force-ims-voice.disabled && mv /var/addons/volte/data/force-ims-voice /var/addons/volte/data/force-ims-voice.disabled'
```

Menedżer po około sekundzie zdejmie zarządzany bit, także bez zmiany LTE/IMS.
Potwierdzić w logu i aktualnej masce. Nie nadpisywać całej maski starą liczbą.
Ponowne włączenie:

```bash
bash tools/bb-ssh 'test ! -e /var/addons/volte/data/force-ims-voice && mv /var/addons/volte/data/force-ims-voice.disabled /var/addons/volte/data/force-ims-voice'
```

Kolejna instalacja paczki przywróci dołączony marker; wyłączenie dotyczy bieżącej
instalacji. Cały dodatek można cofnąć do backupu, po zatrzymaniu menedżera.
Weryfikować osobno pliki, DCA i PPS — samo cofnięcie binarki nie cofa konfiguracji.

Nie testowano numerów alarmowych; ta funkcja nie potwierdza ich działania.
