# Trwała obsługa flagi IMS voice — plan i zapis wykonania

Cel: odtworzyć udany test Orange automatycznie po uruchomieniu Q10, bez pętli
SSH i bez przejmowania całego PR #5. Zgoda użytkownika obejmuje implementację,
instalację i późniejszy test po restarcie; T-Mobile będzie sprawdzane później.

Projekt: `pkg/data/force-ims-voice` włącza eksperymentalne zarządzanie bitem
0x02000000 w status_public. Pętla główna enable co sekundę odczytuje aktualną
maskę, LTE/SIM/data i rejestrację IMS. Zmienia tylko ten bit; czyta też prefiks
[n]. Utrata warunków, usunięcie pliku lub kontrolowane zakończenie zdejmują bit.
Nie zmieniamy flag alarmowych, profili operatorów ani trybu sieci.
SIGKILL/awaria procesu nie dają gwarancji posprzątania; awaria odczytu PPS
jest raportowana i nie upoważnia do wpisania starej maski.

Alternatywy: stała liczba maski nie zachowuje pozostałych bitów; osobny skrypt
autostartu powielałby stan i lifecycle istniejącego menedżera. Wybrano mały
moduł parsowania/polityki i integrację z istniejącą pętlą.

Prace prowadzone w tym samym repo na gałęzi feat/persistent-ims-voice,
bez dodatkowej kopii poza katalogiem wskazanym przez użytkownika.

- [x] Testy hostowe polityki/PPS: [n], brak IMS/LTE, zachowanie innych bitów,
  usunięcie flagi, zła/niepełna liczba, przepełnienie 32-bit, mały bufor.
  Najpierw czerwony test, potem implementacja `src/voice_flag.c/.h`.
- [x] Integracja: synchronizacja co sekundę w enable, także ALREADY_CONNECTED;
  kontrolowane wyjście czyści bit. Marker nie ma skutków ubocznych w probe/check.
- [x] Lifecycle: sygnał tylko zgłasza zatrzymanie; nieblokujący odczyt Berry
  i ustawienie flagi przed pthread_create usuwają znane przeszkody start/stop.
  Enable czeka na gotowość usług po starcie telefonu, zamiast kończyć raz na zawsze.
- [x] Pełne testy, build ARM, niezależny przegląd zmian, dokumentacja wyłączenia.
- [x] Backup telefonu, zatrzymanie starego menedżera, instalacja i porównanie
  hasha nowej binarki; kontrola ustawienia, wyłączenia i ponownego włączenia flagi.
- [ ] Zapis commita. Restart i rozmowy po restarcie oceniane osobno na telefonie.

Testy na telefonie nie obejmują numerów alarmowych. Funkcja umożliwia wybór
głosu IMS, ale nie dowodzi poprawności wszystkich procedur telefonu/sieci.

Wyniki: RED — brak modułu; GREEN — 41 kontroli parsowania/polityki. Po przeglądzie
RED — brak blokady instancji; GREEN — 49 kontroli, w tym rzeczywiste procesy
POSIX/fcntl. Pełne testy: 36 + 49, 0 błędów. Build ARM: kod 0.
Przegląd wykazał potrzebę ponawiania startup cleanup po przejściowej awarii PPS;
dodano ponowienie w pętli oczekiwania. Dodano też blokadę procesu przed pierwszym
zapisem, żeby druga instancja nie zdejmowała flagi pierwszej.

Telefon: marker OFF/ON, odmowa drugiej instancji bez zmiany maski, SIGTERM
z cleanup i ponowny start — potwierdzone. Po wyłączeniu markerem proces przestaje
zarządzać bitem, zamiast stale wymuszać zero; sprawdzono to kontrolowanym wpisem
testowym, następnie cofniętym, i ponownie włączono marker.
Nie wstrzykiwano awarii odczytu PPS ani SIGTERM w trakcie blokującej recovery;
to pozostaje ograniczeniem walidacji, mimo obsługi tych ścieżek w kodzie.
