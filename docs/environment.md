# Środowisko WSL → Windows → BlackBerry

Repozytorium robocze: `/home/alek/projects/own/bb10-volte`.
Źródła bazowe: `baad3eaad0112d5810e5af732703abb2953f37d5`.
Nie zmieniamy profilu Orange ani kodu dodatku podczas przygotowania środowiska.

## Plan i decyzje

- [x] Odczytać wszystkie początkowe pliki `docs/`, stan Git i PR #2 upstream.
- [x] Sprawdzić Windows OpenSSH i klucz klienta bez odczytu jego zawartości.
- [x] Potwierdzić SSH z wymaganą weryfikacją znanego klucza hosta.
- [x] Sprawdzić transport SCP na nieszkodliwym pliku i porównać SHA-256.
- [x] Zbudować niezmieniony dodatek w lokalnym Dockerze i zweryfikować ZIP.
- [x] Zebrać wybrane pola bazowego stanu Orange wyłącznie przez odczyty PPS.

Wybór implementacyjny: cienkie skrypty Bash uruchamiają Windows `ssh.exe`
i `scp.exe` bez PowerShell, dzięki czemu ścieżki są osobnymi argumentami.
Odpowiada to działającej trasie USB Windows; Linux SSH wymagałby osobnego
rozwiązania routingu WSL. PowerShell jest używany tylko do rozpoznania interfejsu.

## Komunikacja

```bash
bash tools/bb-ssh 'uname -a'
bash tools/bb-send ./volte.zip /var/rootdata/volte.zip
bash tools/bb-fetch /var/rootdata/volte.zip ./pobrany-volte.zip
```

Polecenia transferu ZIP są przykładami na później; nie instalują pakietu.
`bb-send` zapisuje plik na telefonie. `bb-fetch` może nadpisać lokalny plik.
Nie przesyłaj nic do plików usług, konfiguracji lub PPS bez osobnej zgody.
Skrypty uruchamiaj przez `bash`, bez konieczności ustawiania executable bit.

Domyślnie: `root@169.254.0.1`, klucz Windows
`D:\blackberry\keys\id_rsa4x`, OpenSSH w
`/mnt/c/Windows/System32/OpenSSH`. Nadpisanie: `BB_HOST`, `BB_KEY_WIN`,
`BB_OPENSSH_DIR`. SCP używa protokołu legacy (`-O`), a ścieżki lokalne są
konwertowane przez `wslpath`. Ścieżki zdalne są bezwzględne, bez spacji
i znaków powłoki; lokalne mogą zawierać spacje.

`bb-ssh` przyjmuje zaufane polecenie powłoki zdalnej. Dla złożonych poleceń
przekazuj jeden argument w apostrofach, aby nie rozwinąć `$` po stronie WSL.

Obowiązują `BatchMode=yes`, `IdentitiesOnly=yes`, `StrictHostKeyChecking=yes`.
Używany jest standardowy Windows `known_hosts`. Przy zmianie klucza po flashu
najpierw porównaj fingerprint z właściwym plikiem publicznym instalacji;
nie wyłączaj weryfikacji. Starsze algorytmy są dopuszczone tylko w tych skryptach.
Nie zapisujemy haseł ani kluczy prywatnych w repozytorium.

Sandbox tego agenta blokuje interop WSL (`UtilBindVsockAnyPort`) i `sudo`.
Wywołania Windows i Docker wymagają wykonania poza sandboxem po zatwierdzeniu.

## Kompilacja i diagnostyka

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

Podany SHA jest lokalnym ID obrazu, nie digestem manifestu do pobierania z GHCR.
`-B` wymusza kompilację ze źródeł. Śledzona binarka `src/ims_manager` nie jest
wynikiem tego Makefile; wynik trafia do `pkg/init/ims_manager`.
Nie wykonuj `make clean` w ciemno ani nie używaj historycznej binarki do instalacji.

Przed instalacją pozostają: potwierdzenie SIM 260/03 i carrier 149,
rejestracji Orange, danych LTE oraz ręczne testy SMS i zwykłych rozmów.
Testy rozmów i instalacja wymagają osobnej zgody. Odczyty logów przed zapisem
należy ograniczać/anonimizować (IMSI, ICCID, IMEI, numery, tożsamości IMS).
